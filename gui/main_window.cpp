#include "main_window.h"
#include "scenario_document.h"
#include "hex_editor_widget.h"
#include "chunk_tree_widget.h"
#include "chunk_properties_widget.h"
#include "high_level_editor_dialog.h"
#include "change_overview_widget.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QApplication>
#include <QDragEnterEvent>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_document(std::make_unique<ScenarioDocument>(this))
    , m_mainSplitter(nullptr)
    , m_chunkTree(nullptr)
    , m_rightTabs(nullptr)
    , m_hexEditor(nullptr)
    , m_propertiesWidget(nullptr)
    , m_dualViewSplitter(nullptr)
    , m_hexEditorDual(nullptr)
    , m_propertiesWidgetDual(nullptr)
{
    setWindowTitle("RoN Scenario Viewer (Read-Only)");
    resize(1400, 900);

    createActions();
    createMenus();
    createToolBar();
    createStatusBar();
    setupLayout();
    setAcceptDrops(true);

    // Connect document signals
    connect(m_document.get(), &ScenarioDocument::dirtyChanged,
            this, &MainWindow::handleDocumentDirtyChanged);
    connect(m_document.get(), &ScenarioDocument::dataLoaded,
            this, &MainWindow::updateWindowTitle);
    connect(m_document.get(), &ScenarioDocument::fileReloaded,
            this, [this](const QString& path) {
                QFileInfo info(path);
                statusBar()->showMessage(
                    tr("File changed on disk, reloaded: %1").arg(info.fileName()),
                    4000);
            });
    connect(m_document.get(), &ScenarioDocument::fileReloadDiff,
            m_hexEditor, &HexEditorWidget::showDiffRanges);
    connect(m_document.get(), &ScenarioDocument::fileReloadDiff,
            m_hexEditorDual, &HexEditorWidget::showDiffRanges);
    connect(m_document.get(), &ScenarioDocument::fileReloadByteChanges,
            m_hexEditor, &HexEditorWidget::showByteChanges);
    connect(m_document.get(), &ScenarioDocument::fileReloadByteChanges,
            m_hexEditorDual, &HexEditorWidget::showByteChanges);
    connect(m_document.get(), &ScenarioDocument::fileReloadFieldChanges,
            m_propertiesWidget, &ChunkPropertiesWidget::setFieldChanges);
    connect(m_document.get(), &ScenarioDocument::fileReloadFieldChanges,
            m_propertiesWidgetDual, &ChunkPropertiesWidget::setFieldChanges);
    connect(m_document.get(), &ScenarioDocument::dataLoaded,
            m_propertiesWidget, &ChunkPropertiesWidget::clear);
    connect(m_document.get(), &ScenarioDocument::dataLoaded,
            m_propertiesWidgetDual, &ChunkPropertiesWidget::clear);
    connect(m_document.get(), &ScenarioDocument::fileReloadFieldChanges,
            m_changeOverview, &ChangeOverviewWidget::setChanges);
    connect(m_document.get(), &ScenarioDocument::dataLoaded,
            m_changeOverview, &ChangeOverviewWidget::clear);
    connect(m_document.get(), &ScenarioDocument::errorOccurred,
            [this](const QString& msg) {
                QMessageBox::critical(this, "Error", msg);
            });

    connect(m_changeOverview, &ChangeOverviewWidget::highlightRequested,
            this, [this](qulonglong offset, qulonglong length) {
                if (m_hexEditor) {
                    m_hexEditor->scrollToOffset(static_cast<size_t>(offset));
                    m_hexEditor->highlightRange(static_cast<size_t>(offset), static_cast<size_t>(length));
                }
                if (m_hexEditorDual) {
                    m_hexEditorDual->scrollToOffset(static_cast<size_t>(offset));
                    m_hexEditorDual->highlightRange(static_cast<size_t>(offset), static_cast<size_t>(length));
                }
            });

    loadSettings();
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    // Destructor defined here to allow unique_ptr with forward declaration
}

void MainWindow::createActions()
{
    m_openAction = new QAction(tr("&Open..."), this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip(tr("Open a scenario file"));
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);

    m_saveAction = new QAction(tr("&Save"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("Save the scenario file"));
    m_saveAction->setEnabled(false);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveFile);

    m_saveAsAction = new QAction(tr("Save &As..."), this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("Save the scenario file with a new name"));
    m_saveAsAction->setEnabled(false);
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);

    m_exitAction = new QAction(tr("E&xit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    m_aboutAction = new QAction(tr("&About"), this);
    m_aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    m_highLevelEditorAction = new QAction(tr("&High-Level Editor..."), this);
    m_highLevelEditorAction->setStatusTip(tr("Open high-level scenario editor"));
    m_highLevelEditorAction->setEnabled(false);
    connect(m_highLevelEditorAction, &QAction::triggered, this, &MainWindow::openHighLevelEditor);
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_openAction);
    fileMenu->addAction(m_saveAction);
    fileMenu->addAction(m_saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(m_highLevelEditorAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::createToolBar()
{
    QToolBar *toolBar = addToolBar(tr("Main"));
    toolBar->addAction(m_openAction);
    toolBar->addAction(m_saveAction);
    toolBar->addSeparator();
    toolBar->addAction(m_highLevelEditorAction);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::setupLayout()
{
    // Create main splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Create chunk tree widget (left side)
    m_chunkTree = new ChunkTreeWidget(m_document.get(), this);
    m_mainSplitter->addWidget(m_chunkTree);

    // Create right side with tabs
    m_rightTabs = new QTabWidget(this);

    // Create hex editor
    m_hexEditor = new HexEditorWidget(m_document.get(), this);
    m_rightTabs->addTab(m_hexEditor, "Hex Editor");

    // Create properties widget
    m_propertiesWidget = new ChunkPropertiesWidget(m_document.get(), this);
    m_rightTabs->addTab(m_propertiesWidget, "Properties");

    // Optional dual view: hex editor and properties side-by-side
    m_dualViewSplitter = new QSplitter(Qt::Horizontal, this);
    m_hexEditorDual = new HexEditorWidget(m_document.get(), m_dualViewSplitter);
    m_propertiesWidgetDual = new ChunkPropertiesWidget(m_document.get(), m_dualViewSplitter);
    m_dualViewSplitter->addWidget(m_hexEditorDual);
    m_dualViewSplitter->addWidget(m_propertiesWidgetDual);
    m_dualViewSplitter->setStretchFactor(0, 6);
    m_dualViewSplitter->setStretchFactor(1, 4);
    m_rightTabs->addTab(m_dualViewSplitter, "Hex + Properties");

    // Changes overview tab
    m_changeOverview = new ChangeOverviewWidget(this);
    m_rightTabs->addTab(m_changeOverview, "Changes");

    m_mainSplitter->addWidget(m_rightTabs);

    // Set splitter sizes (30% tree, 70% editor)
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 7);

    setCentralWidget(m_mainSplitter);

    // Connect signals between widgets
    connect(m_chunkTree, &ChunkTreeWidget::chunkSelected,
            m_hexEditor, &HexEditorWidget::handleChunkSelected);
    connect(m_chunkTree, &ChunkTreeWidget::chunkSelected,
            m_propertiesWidget, &ChunkPropertiesWidget::displayChunk);
    connect(m_chunkTree, &ChunkTreeWidget::chunkSelected,
            m_hexEditorDual, &HexEditorWidget::handleChunkSelected);
    connect(m_chunkTree, &ChunkTreeWidget::chunkSelected,
            m_propertiesWidgetDual, &ChunkPropertiesWidget::displayChunk);
    connect(m_document.get(), &ScenarioDocument::chunkSelected,
            m_propertiesWidget, &ChunkPropertiesWidget::displayChunk);
    connect(m_document.get(), &ScenarioDocument::chunkSelected,
            m_propertiesWidgetDual, &ChunkPropertiesWidget::displayChunk);
    connect(m_document.get(), &ScenarioDocument::chunkSelected,
            m_chunkTree, &ChunkTreeWidget::highlightChunk);
}

void MainWindow::openFile()
{
    if (!maybeSave()) {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Scenario File"), QString(),
        tr("Scenario Files (*.scx);;All Files (*)"));

    if (fileName.isEmpty()) {
        return;
    }

    if (m_document->loadFile(fileName)) {
        m_currentFilePath = fileName;
        // Disabled in minimal read-only build
        // m_saveAction->setEnabled(true);
        // m_saveAsAction->setEnabled(true);
        // m_highLevelEditorAction->setEnabled(true);
        statusBar()->showMessage(tr("File loaded successfully (read-only mode)"), 3000);
    }
}

void MainWindow::saveFile()
{
    if (m_currentFilePath.isEmpty()) {
        saveFileAs();
        return;
    }

    bool compress = true; // Default to compressed
    if (m_currentFilePath.toLower().endsWith(".scx")) {
        compress = true;
    }

    if (m_document->saveFile(m_currentFilePath, compress)) {
        statusBar()->showMessage(tr("File saved successfully"), 3000);
    }
}

void MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Scenario File"), QString(),
        tr("Scenario Files (*.scx);;All Files (*)"));

    if (fileName.isEmpty()) {
        return;
    }

    bool compress = fileName.toLower().endsWith(".scx");

    if (m_document->saveFile(fileName, compress)) {
        m_currentFilePath = fileName;
        updateWindowTitle();
        statusBar()->showMessage(tr("File saved successfully"), 3000);
    }
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("About RoN Scenario Viewer"),
        tr("<h2>Rise of Nations Scenario Viewer</h2>"
           "<p><b>Read-Only Build</b></p>"
           "<p>A GUI tool for viewing .scx scenario files.</p>"
           "<p><b>Current Features:</b></p>"
           "<ul>"
           "<li>Hex viewer with color-coded chunk highlighting</li>"
           "<li>Hierarchical chunk tree display</li>"
           "<li>Field-level property viewing</li>"
           "<li>Search functionality (hex patterns and text)</li>"
           "<li>ASCII/UTF-16 text display toggle</li>"
           "</ul>"
           "<p><b>Coming Soon:</b></p>"
           "<ul>"
           "<li>Hex editing</li>"
           "<li>High-level scenario modifications</li>"
           "<li>Save functionality</li>"
           "</ul>"
           "<p>Built with Qt and the RoN Scenario Parser library.</p>"));
}

void MainWindow::openHighLevelEditor()
{
    // Disabled in minimal build
    QMessageBox::information(this, "Feature Disabled",
        "High-level editing is disabled in this minimal build.\n\n"
        "This is a read-only viewer. Save and edit features will be\n"
        "enabled once the full API integration is complete.");

    /* COMMENTED OUT FOR MINIMAL BUILD
    HighLevelEditorDialog dialog(m_document.get(), this);
    dialog.exec();
    */
}

void MainWindow::updateWindowTitle()
{
    QString title = "RoN Scenario Viewer (Read-Only)";

    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fileInfo(m_currentFilePath);
        title += " - " + fileInfo.fileName();
    }

    // Dirty flag disabled in read-only mode
    // if (m_document->isDirty()) {
    //     title += " *";
    // }

    setWindowTitle(title);
}

void MainWindow::handleDocumentDirtyChanged(bool dirty)
{
    updateWindowTitle();
    m_saveAction->setEnabled(dirty && !m_currentFilePath.isEmpty());
}

void MainWindow::loadSettings()
{
    QSettings settings("RoNScenarioViewer", "MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    m_mainSplitter->restoreState(settings.value("splitterState").toByteArray());
}

void MainWindow::saveSettings()
{
    QSettings settings("RoNScenarioViewer", "MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("splitterState", m_mainSplitter->saveState());
}

bool MainWindow::maybeSave()
{
    if (!m_document->isDirty()) {
        return true;
    }

    QMessageBox::StandardButton ret = QMessageBox::warning(this,
        tr("Unsaved Changes"),
        tr("The document has been modified.\nDo you want to save your changes?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        saveFile();
        return !m_document->isDirty();
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }

    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        saveSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        const auto urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().isLocalFile()) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }

    const auto urls = event->mimeData()->urls();
    if (urls.isEmpty() || !urls.first().isLocalFile()) {
        event->ignore();
        return;
    }

    const QString filePath = urls.first().toLocalFile();
    if (filePath.isEmpty()) {
        event->ignore();
        return;
    }

    // Optional filter: only accept .scx or any file
    // if (!filePath.toLower().endsWith(".scx")) { event->ignore(); return; }

    if (!maybeSave()) {
        event->ignore();
        return;
    }

    if (m_document->loadFile(filePath)) {
        m_currentFilePath = filePath;
        statusBar()->showMessage(tr("File loaded (drop): %1").arg(QFileInfo(filePath).fileName()), 3000);
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}
