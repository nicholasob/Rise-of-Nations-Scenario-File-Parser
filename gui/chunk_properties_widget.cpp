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

void ChunkPropertiesWidget::setFieldChanges(const QVector<FieldChange>& changes)
{
    m_fieldChanges = changes;
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

    // Add element count for variable-length arrays
    if (chunkInfo && chunkInfo->dataSize > 0 && chunk->data.size() >= chunkInfo->dataSize) {
        size_t elementCount = chunk->data.size() / chunkInfo->dataSize;
        if (elementCount > 1) {
            info += QString("<br><b>Array Elements:</b> %1 (each %2 bytes)")
                .arg(elementCount)
                .arg(chunkInfo->dataSize);
        }
    }

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

    // Check if this is a variable-length array chunk
    size_t elementSize = chunkInfo->dataSize;
    size_t elementCount = 1;

    if (elementSize > 0 && chunk->data.size() >= elementSize) {
        elementCount = chunk->data.size() / elementSize;
    }

    // If we have multiple elements, display them all
    if (elementCount > 1) {
        m_table->setRowCount(chunkInfo->fields.size() * elementCount);

        for (size_t elemIdx = 0; elemIdx < elementCount; ++elemIdx) {
            size_t baseOffset = elemIdx * elementSize;

            for (size_t fieldIdx = 0; fieldIdx < chunkInfo->fields.size(); ++fieldIdx) {
                const FieldInfo& field = chunkInfo->fields[fieldIdx];
                size_t rowIdx = elemIdx * chunkInfo->fields.size() + fieldIdx;

                // Field name (with element index for arrays)
                QString fieldName = QString::fromStdString(field.name);
                if (elementCount > 1) {
                    fieldName = QString("[%1] %2").arg(elemIdx).arg(fieldName);
                }
                m_table->setItem(rowIdx, 0, new QTableWidgetItem(fieldName));

                // Type
                m_table->setItem(rowIdx, 1, new QTableWidgetItem(QString::fromStdString(field.type)));

                // Offset (absolute within chunk data)
                size_t absoluteOffset = baseOffset + field.offset;
                m_table->setItem(rowIdx, 2, new QTableWidgetItem(QString::number(absoluteOffset)));

                // Value - decode based on type and size
                QString value;

                if (absoluteOffset + field.size <= chunk->data.size()) {
                    const std::byte* dataPtr = chunk->data.data() + absoluteOffset;

                    // Try to decode value based on type
                    if (field.type == "uint32_t" && field.size == 4) {
                        uint32_t val;
                        std::memcpy(&val, dataPtr, 4);
                        value = QString::number(val);
                    }
                    else if (field.type == "int32_t" && field.size == 4) {
                        int32_t val;
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
                QString oldVal;
                QString newVal;
                if (isFieldChanged(chunk, absoluteOffset, field.size, oldVal, newVal)) {
                    valueItem->setBackground(QBrush(QColor(255, 248, 200))); // soft yellow
                    QString tip = valueItem->toolTip();
                    if (!tip.isEmpty()) {
                        tip += "\n";
                    }
                    tip += QString("Changed: %1 -> %2").arg(oldVal, newVal);
                    valueItem->setToolTip(tip);
                }
                m_table->setItem(rowIdx, 3, valueItem);
            }
        }
    } else {
        // Single element - original behavior
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
                else if (field.type == "int32_t" && field.size == 4) {
                    int32_t val;
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
            QString oldVal;
            QString newVal;
            if (isFieldChanged(chunk, field.offset, field.size, oldVal, newVal)) {
                valueItem->setBackground(QBrush(QColor(255, 248, 200))); // soft yellow
                QString tip = valueItem->toolTip();
                if (!tip.isEmpty()) {
                    tip += "\n";
                }
                tip += QString("Changed: %1 -> %2").arg(oldVal, newVal);
                valueItem->setToolTip(tip);
            }
            m_table->setItem(i, 3, valueItem);
        }
    }
}

bool ChunkPropertiesWidget::isFieldChanged(const Chunk* chunk, size_t absoluteOffset, size_t size, QString& oldVal, QString& newVal) const
{
    const qulonglong start = static_cast<qulonglong>(chunk->file_offset + absoluteOffset);
    const qulonglong end = start + static_cast<qulonglong>(size);

    for (const auto& change : m_fieldChanges) {
        const qulonglong changeStart = change.offset;
        const qulonglong changeEnd = change.offset + change.length;
        if (start < changeEnd && end > changeStart) {
            oldVal = change.oldValue;
            newVal = change.newValue;
            return true;
        }
    }
    return false;
}
