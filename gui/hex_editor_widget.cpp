#include "hex_editor_widget.h"
#include "scenario_document.h"
#include "chunk_metadata.h"
#include "chunk_types.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QMessageBox>
#include <QToolTip>
#include <algorithm>
#include <cctype>

HexEditorWidget::HexEditorWidget(ScenarioDocument *document, QWidget *parent)
    : QWidget(parent)
    , m_document(document)
    , m_utf16Mode(false)
    , m_currentSearchPos(0)
    , m_currentSearchIndex(-1)
    , m_highlightStart(0)
    , m_highlightLength(0)
{
    setupUI();

    connect(m_document, &ScenarioDocument::dataLoaded,
            this, &HexEditorWidget::refresh);
    connect(m_document, &ScenarioDocument::dataChanged,
            this, &HexEditorWidget::refresh);
}

void HexEditorWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Search bar
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Search:", this));

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Enter hex bytes (e.g., 0F 00 00 00) or text");
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &HexEditorWidget::onSearchTextChanged);
    connect(m_searchEdit, &QLineEdit::returnPressed,
            this, &HexEditorWidget::findNext);
    searchLayout->addWidget(m_searchEdit);

    m_findPrevBtn = new QPushButton("Previous", this);
    connect(m_findPrevBtn, &QPushButton::clicked,
            this, &HexEditorWidget::findPrevious);
    searchLayout->addWidget(m_findPrevBtn);

    m_findNextBtn = new QPushButton("Next", this);
    connect(m_findNextBtn, &QPushButton::clicked,
            this, &HexEditorWidget::findNext);
    searchLayout->addWidget(m_findNextBtn);

    m_displayModeCombo = new QComboBox(this);
    m_displayModeCombo->addItem("ASCII");
    m_displayModeCombo->addItem("UTF-16");
    // Qt 5.6 compatible syntax
    connect(m_displayModeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onDisplayModeChanged(int)));
    searchLayout->addWidget(new QLabel("Display:", this));
    searchLayout->addWidget(m_displayModeCombo);

    m_statusLabel = new QLabel(this);
    searchLayout->addWidget(m_statusLabel);
    searchLayout->addStretch();

    mainLayout->addLayout(searchLayout);

    // Hex table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(18); // Offset + 16 bytes + Text

    QStringList headers;
    headers << "Offset";
    for (int i = 0; i < 16; ++i) {
        headers << QString("%1").arg(i, 2, 16, QChar('0')).toUpper();
    }
    headers << "Text";
    m_table->setHorizontalHeaderLabels(headers);

    // Set column widths
    m_table->setColumnWidth(0, 100);  // Offset
    for (int i = 1; i <= 16; ++i) {
        m_table->setColumnWidth(i, 35);  // Hex bytes
    }
    m_table->setColumnWidth(17, 200); // Text

    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setMouseTracking(true); // Enable hover events
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers); // Read-only for minimal build

    // Disabled for minimal build - re-enable for editing support
    // connect(m_table, &QTableWidget::cellChanged,
    //         this, &HexEditorWidget::onCellChanged);

    mainLayout->addWidget(m_table);
}

void HexEditorWidget::refresh()
{
    populateTable();
}

void HexEditorWidget::populateTable()
{
    m_table->blockSignals(true); // Prevent triggering cellChanged during population

    size_t dataSize = m_document->getDataSize();
    if (dataSize == 0) {
        m_table->setRowCount(0);
        m_table->blockSignals(false);
        return;
    }

    int numRows = (dataSize + BYTES_PER_ROW - 1) / BYTES_PER_ROW;
    m_table->setRowCount(numRows);

    for (int row = 0; row < numRows; ++row) {
        updateRow(row);
    }

    m_table->blockSignals(false);
    m_statusLabel->setText(QString("Total size: %1 bytes").arg(dataSize));
}

void HexEditorWidget::updateRow(int row)
{
    size_t baseOffset = row * BYTES_PER_ROW;
    size_t dataSize = m_document->getDataSize();

    // Offset column
    QTableWidgetItem *offsetItem = new QTableWidgetItem(formatOffset(baseOffset));
    offsetItem->setFlags(Qt::ItemIsEnabled);
    offsetItem->setBackground(QBrush(QColor(240, 240, 240)));
    m_table->setItem(row, 0, offsetItem);

    // Hex bytes
    QString textRepr;
    for (int col = 0; col < BYTES_PER_ROW; ++col) {
        size_t offset = baseOffset + col;
        QTableWidgetItem *item = new QTableWidgetItem();

        if (offset < dataSize) {
            uint8_t b = m_document->getByteAt(offset);
            item->setText(byteToHex(b));
            item->setTextAlignment(Qt::AlignCenter);

            // Check if this byte is in highlighted range
            if (offset >= m_highlightStart && offset < m_highlightStart + m_highlightLength) {
                item->setBackground(QBrush(QColor(255, 255, 0, 100))); // Yellow highlight
            } else {
                // Color based on chunk
                const Chunk* chunk = m_document->findChunkContainingOffset(offset);
                if (chunk) {
                    auto& metadata = ChunkMetadata::instance();
                    QColor color = metadata.getColorForChunk(chunk->header.chunk_type_identifier);
                    item->setBackground(QBrush(color));
                }
            }

            // Build text representation
            if (m_utf16Mode && col % 2 == 0 && offset + 1 < dataSize) {
                uint8_t b2 = m_document->getByteAt(offset + 1);
                textRepr += bytesToUtf16(b, b2);
            } else if (!m_utf16Mode) {
                textRepr += byteToAscii(b);
            }
        } else {
            item->setFlags(Qt::ItemIsEnabled);
            item->setBackground(QBrush(QColor(250, 250, 250)));
        }

        m_table->setItem(row, col + 1, item);
    }

    // Text column
    QTableWidgetItem *textItem = new QTableWidgetItem(textRepr);
    textItem->setFlags(Qt::ItemIsEnabled);
    textItem->setFont(QFont("Courier New", 9));
    m_table->setItem(row, 17, textItem);
}

void HexEditorWidget::handleChunkSelected(const Chunk* chunk)
{
    if (!chunk) {
        return;
    }

    scrollToOffset(chunk->file_offset);
    highlightRange(chunk->file_offset, chunk->header.chunk_size);
}

void HexEditorWidget::scrollToOffset(size_t offset)
{
    int row = offset / BYTES_PER_ROW;
    m_table->scrollToItem(m_table->item(row, 0), QAbstractItemView::PositionAtCenter);
}

void HexEditorWidget::highlightRange(size_t start, size_t length)
{
    m_highlightStart = start;
    m_highlightLength = length;
    refresh(); // Re-render to show highlighting
}

void HexEditorWidget::setDisplayMode(bool utf16Mode)
{
    m_utf16Mode = utf16Mode;
    refresh();
}

void HexEditorWidget::onDisplayModeChanged(int index)
{
    setDisplayMode(index == 1); // 0 = ASCII, 1 = UTF-16
}

void HexEditorWidget::onCellChanged(int row, int column)
{
    // Only allow editing hex byte columns (1-16)
    if (column < 1 || column > 16) {
        return;
    }

    size_t offset = row * BYTES_PER_ROW + (column - 1);
    if (offset >= m_document->getDataSize()) {
        return;
    }

    QTableWidgetItem *item = m_table->item(row, column);
    if (!item) {
        return;
    }

    QString text = item->text().trimmed();

    // Validate hex input
    if (text.length() != 2) {
        refresh(); // Reset to original value
        return;
    }

    bool ok;
    uint8_t value = text.toUInt(&ok, 16);
    if (!ok) {
        refresh(); // Reset to original value
        return;
    }

    // Update document
    try {
        m_document->setByteAt(offset, value);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error", QString("Failed to update byte: %1").arg(e.what()));
        refresh();
    }
}

void HexEditorWidget::onSearchTextChanged()
{
    m_searchResults.clear();
    m_currentSearchIndex = -1;

    QString searchText = m_searchEdit->text();
    if (searchText.isEmpty()) {
        m_statusLabel->clear();
        return;
    }

    // Determine search pattern
    std::vector<uint8_t> pattern;

    // Try to parse as hex bytes first
    QStringList hexParts = searchText.split(' ', Qt::SkipEmptyParts);
    bool isHex = true;

    for (const QString& part : hexParts) {
        bool ok;
        uint8_t val = part.toUInt(&ok, 16);
        if (!ok || part.length() > 2) {
            isHex = false;
            break;
        }
        pattern.push_back(val);
    }

    if (!isHex) {
        // Treat as ASCII text
        pattern.clear();
        QByteArray bytes = searchText.toUtf8();
        for (char c : bytes) {
            pattern.push_back(static_cast<uint8_t>(c));
        }
    }

    if (pattern.empty()) {
        return;
    }

    // Search for pattern
    size_t dataSize = m_document->getDataSize();
    for (size_t i = 0; i + pattern.size() <= dataSize; ++i) {
        bool match = true;
        for (size_t j = 0; j < pattern.size(); ++j) {
            if (m_document->getByteAt(i + j) != pattern[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            m_searchResults.push_back(i);
        }
    }

    m_statusLabel->setText(QString("Found %1 matches").arg(m_searchResults.size()));
}

void HexEditorWidget::findNext()
{
    if (m_searchResults.empty()) {
        return;
    }

    m_currentSearchIndex = (m_currentSearchIndex + 1) % m_searchResults.size();
    size_t offset = m_searchResults[m_currentSearchIndex];

    scrollToOffset(offset);
    highlightRange(offset, m_searchEdit->text().split(' ', Qt::SkipEmptyParts).size());

    m_statusLabel->setText(QString("Match %1 of %2")
        .arg(m_currentSearchIndex + 1)
        .arg(m_searchResults.size()));
}

void HexEditorWidget::findPrevious()
{
    if (m_searchResults.empty()) {
        return;
    }

    if (m_currentSearchIndex <= 0) {
        m_currentSearchIndex = m_searchResults.size() - 1;
    } else {
        m_currentSearchIndex--;
    }

    size_t offset = m_searchResults[m_currentSearchIndex];

    scrollToOffset(offset);
    highlightRange(offset, m_searchEdit->text().split(' ', Qt::SkipEmptyParts).size());

    m_statusLabel->setText(QString("Match %1 of %2")
        .arg(m_currentSearchIndex + 1)
        .arg(m_searchResults.size()));
}

void HexEditorWidget::performSearch(bool forward)
{
    Q_UNUSED(forward);
    // Implemented via findNext/findPrevious
}

QString HexEditorWidget::formatOffset(size_t offset) const
{
    return QString("0x%1").arg(offset, 8, 16, QChar('0')).toUpper();
}

QString HexEditorWidget::byteToHex(uint8_t b) const
{
    return QString("%1").arg(b, 2, 16, QChar('0')).toUpper();
}

QChar HexEditorWidget::byteToAscii(uint8_t b) const
{
    if (b >= 32 && b < 127) {
        return QChar(b);
    }
    return QChar('.');
}

QString HexEditorWidget::bytesToUtf16(uint8_t b1, uint8_t b2) const
{
    // Assume little-endian UTF-16
    uint16_t codePoint = static_cast<uint16_t>(b1) | (static_cast<uint16_t>(b2) << 8);

    if (codePoint == 0) {
        return QString(".");
    }

    QChar ch(codePoint);
    if (ch.isPrint()) {
        return QString(ch);
    }

    return QString(".");
}
