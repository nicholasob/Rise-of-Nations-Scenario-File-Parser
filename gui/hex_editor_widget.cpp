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
#include <QTableView>
#include <QAbstractTableModel>
#include <QItemSelectionModel>
#include <algorithm>
#include <cctype>

// Virtualized table model for hex view
class HexTableModel : public QAbstractTableModel
{
public:
    explicit HexTableModel(HexEditorWidget *owner, QObject *parent = nullptr)
        : QAbstractTableModel(parent)
        , m_owner(owner)
    {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid() || !m_owner || !m_owner->m_document) {
            return 0;
        }
        size_t dataSize = m_owner->m_document->getDataSize();
        if (dataSize == 0) {
            return 0;
        }
        return static_cast<int>((dataSize + HexEditorWidget::BYTES_PER_ROW - 1) / HexEditorWidget::BYTES_PER_ROW);
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return 18; // Offset + 16 bytes + Text
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            if (section == 0) return "Offset";
            if (section >= 1 && section <= 16) {
                return QString("%1").arg(section - 1, 2, 16, QChar('0')).toUpper();
            }
            if (section == 17) return "Text";
        }
        return {};
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid() || !m_owner || !m_owner->m_document) {
            return {};
        }

        const int row = index.row();
        const int col = index.column();
        const size_t baseOffset = static_cast<size_t>(row) * HexEditorWidget::BYTES_PER_ROW;
        const size_t dataSize = m_owner->m_document->getDataSize();

        if (role == Qt::DisplayRole) {
            if (col == 0) {
                return m_owner->formatOffset(baseOffset);
            }
            if (col >= 1 && col <= 16) {
                size_t offset = baseOffset + (col - 1);
                if (offset < dataSize) {
                    uint8_t b = m_owner->m_document->getByteAt(offset);
                    return m_owner->byteToHex(b);
                }
                return QString();
            }
            if (col == 17) {
                QString textRepr;
                for (int i = 0; i < HexEditorWidget::BYTES_PER_ROW; ++i) {
                    size_t offset = baseOffset + i;
                    if (offset >= dataSize) break;
                    uint8_t b = m_owner->m_document->getByteAt(offset);
                    if (m_owner->m_utf16Mode && i % 2 == 0 && offset + 1 < dataSize) {
                        uint8_t b2 = m_owner->m_document->getByteAt(offset + 1);
                        textRepr += m_owner->bytesToUtf16(b, b2);
                    } else if (!m_owner->m_utf16Mode) {
                        textRepr += m_owner->byteToAscii(b);
                    }
                }
                return textRepr;
            }
        }

        if (role == Qt::TextAlignmentRole) {
            if (col == 0) return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
            if (col >= 1 && col <= 16) return QVariant(Qt::AlignCenter);
        }

        if (role == Qt::BackgroundRole && col >= 1 && col <= 16) {
            size_t offset = baseOffset + (col - 1);
            if (offset < dataSize) {
                // Diff highlighting
                for (const auto& range : m_owner->m_diffRanges) {
                    const auto start = static_cast<size_t>(range.first);
                    const auto len = static_cast<size_t>(range.second);
                    if (offset >= start && offset < start + len) {
                        return QBrush(QColor(255, 99, 71, 140)); // Tomato tint
                    }
                }

                // Selection highlight
                if (offset >= m_owner->m_highlightStart &&
                    offset < m_owner->m_highlightStart + m_owner->m_highlightLength) {
                    return QBrush(QColor(255, 255, 0, 100)); // Yellow
                }

                // Chunk color
                const Chunk* chunk = m_owner->m_document->findChunkContainingOffset(offset);
                if (chunk) {
                    auto& metadata = ChunkMetadata::instance();
                    QColor color = metadata.getColorForChunk(chunk->header.chunk_type_identifier);
                    return QBrush(color);
                }
            }
        }

        if (role == Qt::ToolTipRole && col >= 1 && col <= 16) {
            size_t offset = baseOffset + (col - 1);
            if (offset < dataSize && m_owner->m_byteChanges.contains(static_cast<qulonglong>(offset))) {
                const auto pair = m_owner->m_byteChanges.value(static_cast<qulonglong>(offset));
                QString tip = "Changed byte ";
                if (pair.first >= 0) {
                    tip += m_owner->byteToHex(static_cast<uint8_t>(pair.first));
                } else {
                    tip += "<none>";
                }
                tip += " -> ";
                if (pair.second >= 0) {
                    tip += m_owner->byteToHex(static_cast<uint8_t>(pair.second));
                } else {
                    tip += "<removed>";
                }
                return tip;
            }
        }

        return {};
    }

    void refresh()
    {
        beginResetModel();
        endResetModel();
    }

private:
    HexEditorWidget *m_owner;
};

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
    connect(m_document, &ScenarioDocument::fileReloadDiff,
            this, &HexEditorWidget::showDiffRanges);
    connect(m_document, &ScenarioDocument::dataLoaded,
            this, &HexEditorWidget::clearDiffHighlight);
    connect(m_document, &ScenarioDocument::fileReloadByteChanges,
            this, &HexEditorWidget::showByteChanges);
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

    // Hex table (virtualized)
    m_model = new HexTableModel(this, this);
    m_table = new QTableView(this);
    m_table->setModel(m_model);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setMouseTracking(true);
    m_table->horizontalHeader()->setDefaultSectionSize(40);
    m_table->setColumnWidth(0, 100);
    for (int i = 1; i <= 16; ++i) {
        m_table->setColumnWidth(i, 35);
    }
    m_table->setColumnWidth(17, 200);
    connect(m_table, &QTableView::clicked,
            this, [this](const QModelIndex &index) {
                handleCellClicked(index.row(), index.column());
            });

    mainLayout->addWidget(m_table);
}

void HexEditorWidget::refresh()
{
    m_byteChanges.clear(); // reset tooltips/highlights on manual reloads
    populateTable();
}

void HexEditorWidget::populateTable()
{
    size_t dataSize = m_document->getDataSize();
    if (m_model) {
        m_model->refresh();
    }
    m_statusLabel->setText(QString("Total size: %1 bytes").arg(dataSize));
}

void HexEditorWidget::handleChunkSelected(const Chunk* chunk)
{
    if (!chunk) {
        return;
    }

    scrollToOffset(chunk->file_offset);
    highlightRange(chunk->file_offset, chunk->header.chunk_size);
}

void HexEditorWidget::handleCellClicked(int row, int column)
{
    // Column 0 is the offset label; columns 1-16 are bytes
    if (column <= 0 || column > BYTES_PER_ROW) {
        return;
    }
    size_t offset = static_cast<size_t>(row) * BYTES_PER_ROW + (column - 1);
    if (offset >= m_document->getDataSize()) {
        return;
    }

    const Chunk* chunk = m_document->findChunkContainingOffset(offset);
    if (!chunk) {
        return;
    }

    m_document->selectChunk(chunk);
    highlightRange(chunk->file_offset, chunk->header.chunk_size);
    emit byteOffsetSelected(static_cast<qulonglong>(offset));
}

void HexEditorWidget::showDiffRanges(const QVector<QPair<qulonglong, qulonglong>>& ranges)
{
    m_diffRanges = ranges;

    int totalBytes = 0;
    for (const auto& r : m_diffRanges) {
        totalBytes += static_cast<int>(r.second);
    }

    if (!m_diffRanges.isEmpty()) {
        m_statusLabel->setText(
            QString("Reloaded: %1 changed regions, %2 bytes")
                .arg(m_diffRanges.size())
                .arg(totalBytes));
    }

    populateTable();
}

void HexEditorWidget::clearDiffHighlight()
{
    if (m_diffRanges.isEmpty()) {
        return;
    }
    m_diffRanges.clear();
    populateTable();
}

void HexEditorWidget::showByteChanges(const QVector<ByteChange>& changes)
{
    m_byteChanges.clear();
    for (const auto& change : changes) {
        m_byteChanges.insert(change.offset, qMakePair(change.oldValue, change.newValue));
    }
    populateTable();
}

void HexEditorWidget::scrollToOffset(size_t offset)
{
    int row = offset / BYTES_PER_ROW;
    if (m_model && m_table && row < m_model->rowCount()) {
        QModelIndex idx = m_model->index(row, 0);
        m_table->scrollTo(idx, QAbstractItemView::PositionAtCenter);
        m_table->setCurrentIndex(idx);
    }
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
