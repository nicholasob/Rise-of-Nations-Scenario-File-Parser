#include "chunk_properties_widget.h"
#include "scenario_document.h"
#include "chunk_types.h"
#include "chunk_metadata.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <cstring>

ChunkPropertiesWidget::ChunkPropertiesWidget(ScenarioDocument *document, QWidget *parent)
    : QWidget(parent)
    , m_document(document)
{
    setupUI();
}

void ChunkPropertiesWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_chunkInfoLabel = new QLabel(this);
    m_chunkInfoLabel->setWordWrap(true);
    m_chunkInfoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 8px; border: 1px solid #ccc; }");
    layout->addWidget(m_chunkInfoLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({"Field Name", "Type", "Offset", "Value"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(0, 200);
    m_table->setColumnWidth(1, 150);
    m_table->setColumnWidth(2, 80);
    m_table->setAlternatingRowColors(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

    layout->addWidget(m_table);
}

void ChunkPropertiesWidget::displayChunk(const Chunk* chunk)
{
    if (!chunk) {
        clear();
        return;
    }

    displayHeader(chunk);
    displayFields(chunk);
}

void ChunkPropertiesWidget::clear()
{
    m_chunkInfoLabel->clear();
    m_table->setRowCount(0);
}

void ChunkPropertiesWidget::displayHeader(const Chunk* chunk)
{
    auto& metadata = ChunkMetadata::instance();
    const auto* chunkInfo = metadata.getChunkInfo(chunk->header.chunk_type_identifier);
    std::string typeName = chunkInfo ? chunkInfo->name : ("UNKNOWN_" + std::to_string(static_cast<uint16_t>(chunk->header.chunk_type_identifier)));
    std::string category = metadata.getCategory(chunk->header.chunk_type_identifier);

    QString info = QString("<b>Chunk Type:</b> %1 (0x%2)<br>")
        .arg(QString::fromStdString(typeName))
        .arg(static_cast<uint16_t>(chunk->header.chunk_type_identifier), 4, 16, QChar('0')).toUpper();

    info += QString("<b>Category:</b> %1<br>").arg(QString::fromStdString(category));
    info += QString("<b>File Offset:</b> 0x%1<br>").arg(chunk->file_offset, 8, 16, QChar('0')).toUpper();
    info += QString("<b>Chunk Size:</b> %1 bytes<br>").arg(chunk->header.chunk_size);
    info += QString("<b>Children Count:</b> %1<br>").arg(chunk->children.size());
    info += QString("<b>Data Size:</b> %1 bytes").arg(chunk->data.size());

    m_chunkInfoLabel->setText(info);
}

void ChunkPropertiesWidget::displayFields(const Chunk* chunk)
{
    m_table->setRowCount(0);

    auto& metadata = ChunkMetadata::instance();
    const ChunkInfo* chunkInfo = metadata.getChunkInfo(chunk->header.chunk_type_identifier);

    if (!chunkInfo || chunkInfo->fields.empty()) {
        // No field metadata available
        m_table->setRowCount(1);
        m_table->setItem(0, 0, new QTableWidgetItem("No field metadata available"));
        m_table->setSpan(0, 0, 1, 4);
        return;
    }

    m_table->setRowCount(chunkInfo->fields.size());

    for (size_t i = 0; i < chunkInfo->fields.size(); ++i) {
        const FieldInfo& field = chunkInfo->fields[i];

        // Field name
        m_table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(field.name)));

        // Type
        m_table->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(field.type)));

        // Offset (relative to chunk data)
        m_table->setItem(i, 2, new QTableWidgetItem(QString::number(field.offset)));

        // Value - decode based on type and size
        QString value;

        if (field.offset + field.size <= chunk->data.size()) {
            const std::byte* dataPtr = chunk->data.data() + field.offset;

            // Try to decode value based on type
            if (field.type == "uint32_t" && field.size == 4) {
                uint32_t val;
                std::memcpy(&val, dataPtr, 4);
                value = QString::number(val);
            }
            else if (field.type == "uint16_t" && field.size == 2) {
                uint16_t val;
                std::memcpy(&val, dataPtr, 2);
                value = QString::number(val);
            }
            else if (field.type == "uint8_t" && field.size == 1) {
                uint8_t val = static_cast<uint8_t>(*dataPtr);
                value = QString::number(val);
            }
            else if (field.type.find("char16_t") != std::string::npos) {
                // UTF-16 string
                std::u16string str;
                for (size_t j = 0; j < field.size / 2; ++j) {
                    char16_t ch;
                    std::memcpy(&ch, dataPtr + j * 2, 2);
                    if (ch == 0) break;
                    str += ch;
                }
                value = QString::fromStdU16String(str);
            }
            else {
                // Show as hex bytes
                QStringList hexBytes;
                for (size_t j = 0; j < std::min(field.size, size_t(16)); ++j) {
                    hexBytes << QString("%1").arg(static_cast<uint8_t>(dataPtr[j]), 2, 16, QChar('0')).toUpper();
                }
                if (field.size > 16) {
                    hexBytes << "...";
                }
                value = hexBytes.join(" ");
            }
        } else {
            value = "<out of range>";
        }

        QTableWidgetItem *valueItem = new QTableWidgetItem(value);
        if (!field.description.empty()) {
            valueItem->setToolTip(QString::fromStdString(field.description));
        }
        m_table->setItem(i, 3, valueItem);
    }
}
