#ifndef HIGH_LEVEL_EDITOR_DIALOG_H
#define HIGH_LEVEL_EDITOR_DIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>

class ScenarioDocument;

/**
 * @brief Dialog for high-level scenario modifications
 */
class HighLevelEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HighLevelEditorDialog(ScenarioDocument *document, QWidget *parent = nullptr);
    ~HighLevelEditorDialog() = default;

private slots:
    void applyChanges();
    void loadPlayerData();

private:
    void setupUI();
    void createPlayerTab();
    void createMapTab();

    ScenarioDocument *m_document;
    QTabWidget *m_tabs;
    QTableWidget *m_playerTable;
    QPushButton *m_applyBtn;
    QPushButton *m_closeBtn;
};

#endif // HIGH_LEVEL_EDITOR_DIALOG_H
