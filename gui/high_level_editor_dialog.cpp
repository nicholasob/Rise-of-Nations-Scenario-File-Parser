#include "high_level_editor_dialog.h"
#include "scenario_document.h"
#include "scenario_modifier.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>

HighLevelEditorDialog::HighLevelEditorDialog(ScenarioDocument *document, QWidget *parent)
    : QDialog(parent)
    , m_document(document)
{
    setWindowTitle("High-Level Scenario Editor");
    resize(800, 600);

    setupUI();
    loadPlayerData();
}

void HighLevelEditorDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_tabs = new QTabWidget(this);
    createPlayerTab();
    createMapTab();

    mainLayout->addWidget(m_tabs);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_applyBtn = new QPushButton("Apply Changes", this);
    connect(m_applyBtn, &QPushButton::clicked,
            this, &HighLevelEditorDialog::applyChanges);
    buttonLayout->addWidget(m_applyBtn);

    m_closeBtn = new QPushButton("Close", this);
    connect(m_closeBtn, &QPushButton::clicked,
            this, &QDialog::accept);
    buttonLayout->addWidget(m_closeBtn);

    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
}

void HighLevelEditorDialog::createPlayerTab()
{
    QWidget *playerTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(playerTab);

    QLabel *info = new QLabel("Modify player properties. Note: Some changes may require the scenario to be reloaded.", this);
    info->setWordWrap(true);
    layout->addWidget(info);

    m_playerTable = new QTableWidget(this);
    m_playerTable->setColumnCount(6);
    m_playerTable->setHorizontalHeaderLabels({"Player #", "Name", "Color", "Nation", "Difficulty", "Control"});
    m_playerTable->horizontalHeader()->setStretchLastSection(true);
    m_playerTable->setColumnWidth(0, 80);
    m_playerTable->setColumnWidth(1, 200);
    m_playerTable->setColumnWidth(2, 100);
    m_playerTable->setColumnWidth(3, 120);
    m_playerTable->setColumnWidth(4, 100);

    layout->addWidget(m_playerTable);

    m_tabs->addTab(playerTab, "Players");
}

void HighLevelEditorDialog::createMapTab()
{
    QWidget *mapTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(mapTab);

    QLabel *info = new QLabel("Map information:", this);
    layout->addWidget(info);

    // Get map dimensions
    ScenarioEditor* editor = m_document->getEditor();
    if (editor && editor->IsLoaded()) {
        ScenarioModifier modifier = editor->GetModifier();
        uint32_t width = 0, height = 0;
        if (modifier.GetMapDimensions(width, height)) {
            QString mapInfo = QString("Map Size: %1 x %2 tiles").arg(width).arg(height);
            QLabel *mapLabel = new QLabel(mapInfo, this);
            layout->addWidget(mapLabel);
        }
    }

    layout->addStretch();

    m_tabs->addTab(mapTab, "Map");
}

void HighLevelEditorDialog::loadPlayerData()
{
    ScenarioEditor* editor = m_document->getEditor();
    if (!editor || !editor->IsLoaded()) {
        return;
    }

    ScenarioModifier modifier = editor->GetModifier();
    uint32_t playerCount = modifier.GetPlayerCount();

    m_playerTable->setRowCount(playerCount);

    // Note: Full player loading would require accessing internal data structures
    // For now, we'll create editable rows
    for (uint32_t i = 0; i < playerCount; ++i) {
        // Player number (non-editable)
        QTableWidgetItem *numItem = new QTableWidgetItem(QString::number(i + 1));
        numItem->setFlags(Qt::ItemIsEnabled);
        m_playerTable->setItem(i, 0, numItem);

        // Name (editable)
        QTableWidgetItem *nameItem = new QTableWidgetItem("Player " + QString::number(i + 1));
        m_playerTable->setItem(i, 1, nameItem);

        // Color (combo box)
        QComboBox *colorCombo = new QComboBox(this);
        colorCombo->addItems({"Red", "Blue", "Green", "Yellow", "Cyan", "Purple", "Orange", "White"});
        m_playerTable->setCellWidget(i, 2, colorCombo);

        // Nation (combo box)
        QComboBox *nationCombo = new QComboBox(this);
        nationCombo->addItems({"Random", "Americans", "British", "Chinese", "Egyptians", "French",
                               "Germans", "Greeks", "Inca", "Japanese", "Koreans", "Mayans",
                               "Mongols", "Nubians", "Romans", "Russians", "Spanish", "Turks"});
        m_playerTable->setCellWidget(i, 3, nationCombo);

        // Difficulty (combo box)
        QComboBox *diffCombo = new QComboBox(this);
        diffCombo->addItems({"Easiest", "Easy", "Moderate", "Hard", "Harder", "Hardest", "Toughest"});
        m_playerTable->setCellWidget(i, 4, diffCombo);

        // Control (combo box)
        QComboBox *controlCombo = new QComboBox(this);
        controlCombo->addItems({"Human", "Computer"});
        m_playerTable->setCellWidget(i, 5, controlCombo);
    }
}

void HighLevelEditorDialog::applyChanges()
{
    ScenarioEditor* editor = m_document->getEditor();
    if (!editor || !editor->IsLoaded()) {
        QMessageBox::warning(this, "Error", "No scenario editor available");
        return;
    }

    ScenarioModifier modifier = editor->GetModifier();

    // Apply player modifications
    for (int i = 0; i < m_playerTable->rowCount(); ++i) {
        // Get name
        QTableWidgetItem *nameItem = m_playerTable->item(i, 1);
        if (nameItem) {
            QString name = nameItem->text();
            std::u16string u16name = name.toStdU16String();
            modifier.SetPlayerName(i, u16name);
        }

        // Get color
        QComboBox *colorCombo = qobject_cast<QComboBox*>(m_playerTable->cellWidget(i, 2));
        if (colorCombo) {
            uint8_t colorIndex = colorCombo->currentIndex();
            modifier.SetPlayerColor(i, colorIndex);
        }

        // Get nation
        QComboBox *nationCombo = qobject_cast<QComboBox*>(m_playerTable->cellWidget(i, 3));
        if (nationCombo) {
            uint32_t nationIndex = nationCombo->currentIndex();
            modifier.SetPlayerNation(i, nationIndex);
        }

        // Get difficulty
        QComboBox *diffCombo = qobject_cast<QComboBox*>(m_playerTable->cellWidget(i, 4));
        if (diffCombo) {
            uint32_t difficulty = diffCombo->currentIndex();
            modifier.SetPlayerDifficulty(i, difficulty);
        }

        // Get control
        QComboBox *controlCombo = qobject_cast<QComboBox*>(m_playerTable->cellWidget(i, 5));
        if (controlCombo) {
            bool isHuman = (controlCombo->currentIndex() == 0);
            modifier.SetPlayerControl(i, isHuman);
        }
    }

    // Refresh document from editor
    m_document->refreshFromEditor();

    QMessageBox::information(this, "Success",
        "Changes applied successfully. The hex view and chunk tree have been updated.");
}
