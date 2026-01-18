#ifndef HEX_EDITOR_WIDGET_H
#define HEX_EDITOR_WIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QTableView>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QHash>
#include <vector>
#include <cstddef>
#include <QVector>
#include <QPair>

class ScenarioDocument;
struct Chunk;
struct ByteChange;

class HexEditorWidget;
class HexTableModel;

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
    void showDiffRanges(const QVector<QPair<qulonglong, qulonglong>>& ranges);
    void clearDiffHighlight();
    void showByteChanges(const QVector<ByteChange>& changes);
    void findNext();
    void findPrevious();
    void handleCellClicked(int row, int column);

private slots:
    void onSearchTextChanged();
    void onDisplayModeChanged(int index);

private:
    friend class HexTableModel;
    void setupUI();
    void populateTable();
    void performSearch(bool forward);
    QString formatOffset(size_t offset) const;
    QString byteToHex(uint8_t b) const;
    QChar byteToAscii(uint8_t b) const;
    QString bytesToUtf16(uint8_t b1, uint8_t b2) const;

    ScenarioDocument *m_document;
    HexTableModel *m_model;
    QTableView *m_table;
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
    QVector<QPair<qulonglong, qulonglong>> m_diffRanges;
    QHash<qulonglong, QPair<int,int>> m_byteChanges; // offset -> <old,new>

    static const int BYTES_PER_ROW = 16;
};

#endif // HEX_EDITOR_WIDGET_H
