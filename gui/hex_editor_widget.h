#ifndef HEX_EDITOR_WIDGET_H
#define HEX_EDITOR_WIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <vector>
#include <cstddef>

class ScenarioDocument;
struct Chunk;

/**
 * @brief Hex editor widget with color highlighting and search
 */
class HexEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HexEditorWidget(ScenarioDocument *document, QWidget *parent = nullptr);
    ~HexEditorWidget() = default;

    void scrollToOffset(size_t offset);
    void highlightRange(size_t start, size_t length);
    void setDisplayMode(bool utf16Mode);

public slots:
    void refresh();
    void handleChunkSelected(const Chunk* chunk);
    void findNext();
    void findPrevious();

private slots:
    void onCellChanged(int row, int column);
    void onSearchTextChanged();
    void onDisplayModeChanged(int index);

private:
    void setupUI();
    void populateTable();
    void updateRow(int row);
    void performSearch(bool forward);
    QString formatOffset(size_t offset) const;
    QString byteToHex(uint8_t b) const;
    QChar byteToAscii(uint8_t b) const;
    QString bytesToUtf16(uint8_t b1, uint8_t b2) const;

    ScenarioDocument *m_document;
    QTableWidget *m_table;
    QLineEdit *m_searchEdit;
    QPushButton *m_findNextBtn;
    QPushButton *m_findPrevBtn;
    QComboBox *m_displayModeCombo;
    QLabel *m_statusLabel;

    bool m_utf16Mode;
    size_t m_currentSearchPos;
    std::vector<size_t> m_searchResults;
    int m_currentSearchIndex;
    size_t m_highlightStart;
    size_t m_highlightLength;

    static const int BYTES_PER_ROW = 16;
};

#endif // HEX_EDITOR_WIDGET_H
