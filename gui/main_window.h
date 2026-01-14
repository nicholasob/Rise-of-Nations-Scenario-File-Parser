#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include <memory>

class ScenarioDocument;
class HexEditorWidget;
class ChunkTreeWidget;
class ChunkPropertiesWidget;

/**
 * @brief Main application window for scenario viewer
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow(); // Defined in .cpp to allow unique_ptr with forward declaration

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void openFile();
    void saveFile();
    void saveFileAs();
    void showAbout();
    void openHighLevelEditor();
    void updateWindowTitle();
    void handleDocumentDirtyChanged(bool dirty);

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void createStatusBar();
    void createDockWidgets();
    void setupLayout();
    void loadSettings();
    void saveSettings();
    bool maybeSave();

    // Widgets
    std::unique_ptr<ScenarioDocument> m_document;
    QSplitter *m_mainSplitter;
    ChunkTreeWidget *m_chunkTree;
    QTabWidget *m_rightTabs;
    HexEditorWidget *m_hexEditor;
    ChunkPropertiesWidget *m_propertiesWidget;

    // Actions
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_saveAsAction;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    QAction *m_highLevelEditorAction;

    // State
    QString m_currentFilePath;
};

#endif // MAIN_WINDOW_H
