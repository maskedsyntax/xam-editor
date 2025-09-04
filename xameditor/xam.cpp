#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtGui/QFont>
#include <QtGui/QKeySequence>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <memory>

class FindReplaceDialog : public QDialog {
    Q_OBJECT

  private:
    QLineEdit *findLineEdit;
    QLineEdit *replaceLineEdit;
    QPushButton *findNextButton;
    QPushButton *findPrevButton;
    QPushButton *replaceButton;
    QPushButton *replaceAllButton;
    QCheckBox *caseSensitiveBox;
    QCheckBox *wholeWordsBox;
    QTextEdit *textEdit;
    QTextCursor lastFoundCursor;
    bool isReplaceMode;

  public:
    FindReplaceDialog(QTextEdit *editor, bool replaceMode = false, QWidget *parent = nullptr)
        : QDialog(parent), textEdit(editor), isReplaceMode(replaceMode) {
        setWindowTitle(replaceMode ? "Find and Replace" : "Find");
        setModal(false);
        setupUI();
        connectSignals();
    }

  private slots:
    void findNext() {
        QString searchText = findLineEdit->text();
        if (searchText.isEmpty())
            return;

        QTextDocument::FindFlags flags = QTextDocument::FindFlags();
        if (caseSensitiveBox->isChecked())
            flags |= QTextDocument::FindCaseSensitively;
        if (wholeWordsBox->isChecked())
            flags |= QTextDocument::FindWholeWords;

        QTextCursor cursor = textEdit->textCursor();
        cursor = textEdit->document()->find(searchText, cursor, flags);

        if (cursor.isNull()) {
            // Search from beginning
            cursor = textEdit->document()->find(searchText, 0, flags);
        }

        if (!cursor.isNull()) {
            textEdit->setTextCursor(cursor);
            lastFoundCursor = cursor;
        } else {
            QMessageBox::information(this, "Find", "Text not found.");
        }
    }

    void findPrevious() {
        QString searchText = findLineEdit->text();
        if (searchText.isEmpty())
            return;

        QTextDocument::FindFlags flags = QTextDocument::FindBackward;
        if (caseSensitiveBox->isChecked())
            flags |= QTextDocument::FindCaseSensitively;
        if (wholeWordsBox->isChecked())
            flags |= QTextDocument::FindWholeWords;

        QTextCursor cursor = textEdit->textCursor();
        cursor = textEdit->document()->find(searchText, cursor, flags);

        if (cursor.isNull()) {
            // Search from end
            cursor = textEdit->document()->find(searchText, textEdit->document()->characterCount(),
                                                flags);
        }

        if (!cursor.isNull()) {
            textEdit->setTextCursor(cursor);
            lastFoundCursor = cursor;
        } else {
            QMessageBox::information(this, "Find", "Text not found.");
        }
    }

    void replace() {
        if (!lastFoundCursor.isNull() && lastFoundCursor.hasSelection()) {
            lastFoundCursor.insertText(replaceLineEdit->text());
            textEdit->setTextCursor(lastFoundCursor);
        }
        findNext();
    }

    void replaceAll() {
        QString searchText = findLineEdit->text();
        QString replaceText = replaceLineEdit->text();
        if (searchText.isEmpty())
            return;

        QTextDocument::FindFlags flags = QTextDocument::FindFlags();
        if (caseSensitiveBox->isChecked())
            flags |= QTextDocument::FindCaseSensitively;
        if (wholeWordsBox->isChecked())
            flags |= QTextDocument::FindWholeWords;

        QTextCursor cursor = textEdit->document()->find(searchText, 0, flags);
        int replacements = 0;

        while (!cursor.isNull()) {
            cursor.insertText(replaceText);
            replacements++;
            cursor = textEdit->document()->find(searchText, cursor, flags);
        }

        QMessageBox::information(this, "Replace All",
                                 QString("Replaced %1 occurrence(s).").arg(replacements));
    }

  private:
    void setupUI() {
        QVBoxLayout *layout = new QVBoxLayout(this);

        // Find section
        QHBoxLayout *findLayout = new QHBoxLayout();
        findLayout->addWidget(new QLabel("Find:"));
        findLineEdit = new QLineEdit();
        findLayout->addWidget(findLineEdit);
        layout->addLayout(findLayout);

        // Replace section (only in replace mode)
        if (isReplaceMode) {
            QHBoxLayout *replaceLayout = new QHBoxLayout();
            replaceLayout->addWidget(new QLabel("Replace:"));
            replaceLineEdit = new QLineEdit();
            replaceLayout->addWidget(replaceLineEdit);
            layout->addLayout(replaceLayout);
        }

        // Options
        QHBoxLayout *optionsLayout = new QHBoxLayout();
        caseSensitiveBox = new QCheckBox("Case sensitive");
        wholeWordsBox = new QCheckBox("Whole words");
        optionsLayout->addWidget(caseSensitiveBox);
        optionsLayout->addWidget(wholeWordsBox);
        layout->addLayout(optionsLayout);

        // Buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        findNextButton = new QPushButton("Find Next");
        findPrevButton = new QPushButton("Find Previous");
        buttonLayout->addWidget(findNextButton);
        buttonLayout->addWidget(findPrevButton);

        if (isReplaceMode) {
            replaceButton = new QPushButton("Replace");
            replaceAllButton = new QPushButton("Replace All");
            buttonLayout->addWidget(replaceButton);
            buttonLayout->addWidget(replaceAllButton);
        }

        QPushButton *closeButton = new QPushButton("Close");
        buttonLayout->addWidget(closeButton);
        layout->addLayout(buttonLayout);

        connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    }

    void connectSignals() {
        connect(findNextButton, &QPushButton::clicked, this, &FindReplaceDialog::findNext);
        connect(findPrevButton, &QPushButton::clicked, this, &FindReplaceDialog::findPrevious);
        connect(findLineEdit, &QLineEdit::returnPressed, this, &FindReplaceDialog::findNext);

        if (isReplaceMode) {
            connect(replaceButton, &QPushButton::clicked, this, &FindReplaceDialog::replace);
            connect(replaceAllButton, &QPushButton::clicked, this, &FindReplaceDialog::replaceAll);
            connect(replaceLineEdit, &QLineEdit::returnPressed, this, &FindReplaceDialog::replace);
        }
    }
};

class XamEditor : public QMainWindow {
    Q_OBJECT

  private:
    QTabWidget *tabWidget;
    int tabCounter;
    bool isDarkTheme;
    FindReplaceDialog *findDialog;
    FindReplaceDialog *replaceDialog;

    struct Tab {
        QTextEdit *textEdit;
        QString filename;
        bool isModified;

        Tab() : textEdit(nullptr), filename("Untitled"), isModified(false) {}
    };

    std::vector<std::unique_ptr<Tab>> tabs;

  public:
    XamEditor(QWidget *parent = nullptr)
        : QMainWindow(parent), tabCounter(1), isDarkTheme(false), findDialog(nullptr),
          replaceDialog(nullptr) {
        setWindowTitle("XamEditor");
        setMinimumSize(800, 600);

        setupUI();
        setupMenus();
        applyTheme();
        newFile();
    }

  private slots:
    void newFile() {
        auto tab = std::make_unique<Tab>();
        tab->textEdit = new QTextEdit();
        tab->textEdit->setFont(QFont("Consolas", 11));
        tab->filename = QString("Untitled %1").arg(tabCounter++);

        connect(tab->textEdit, &QTextEdit::textChanged, [this, tabPtr = tab.get()]() {
            tabPtr->isModified = true;
            updateTabTitle(tabPtr);
        });

        int index = tabWidget->addTab(tab->textEdit, tab->filename);
        tabWidget->setCurrentIndex(index);

        // Save pointer before moving
        QTextEdit *editor = tab->textEdit;

        tabs.push_back(std::move(tab));

        editor->setFocus(); // safe now
    }

    void openFile() {
        QString filename = QFileDialog::getOpenFileName(this, "Open File");
        if (filename.isEmpty())
            return;

        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Error", "Could not open file for reading.");
            return;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        newFile();
        Tab *currentTab = getCurrentTab();
        if (currentTab) {
            currentTab->textEdit->setPlainText(content);
            currentTab->filename = filename;
            currentTab->isModified = false;

            QFileInfo fileInfo(filename);
            tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());
        }
    }

    void saveFile() {
        Tab *tab = getCurrentTab();
        if (!tab)
            return;

        if (tab->filename.startsWith("Untitled")) {
            saveFileAs();
            return;
        }

        QFile file(tab->filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Error", "Could not open file for writing.");
            return;
        }

        QTextStream out(&file);
        out << tab->textEdit->toPlainText();
        file.close();

        tab->isModified = false;
        updateTabTitle(tab);
        statusBar()->showMessage("File saved successfully", 2000);
    }

    void saveFileAs() {
        Tab *tab = getCurrentTab();
        if (!tab)
            return;

        QString filename = QFileDialog::getSaveFileName(this, "Save File As");
        if (filename.isEmpty())
            return;

        tab->filename = filename;
        saveFile();

        QFileInfo fileInfo(filename);
        tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());
    }

    void closeTab() {
        int index = tabWidget->currentIndex();
        if (index == -1)
            return;

        Tab *tab = getCurrentTab();
        if (tab && tab->isModified) {
            int ret = QMessageBox::question(
                this, "Unsaved Changes",
                "The document has unsaved changes. Do you want to save before closing?",
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

            if (ret == QMessageBox::Save) {
                saveFile();
            } else if (ret == QMessageBox::Cancel) {
                return;
            }
        }

        tabWidget->removeTab(index);
        tabs.erase(tabs.begin() + index);

        if (tabs.empty()) {
            newFile();
        }
    }

    void cut() {
        Tab *tab = getCurrentTab();
        if (tab)
            tab->textEdit->cut();
    }

    void copy() {
        Tab *tab = getCurrentTab();
        if (tab)
            tab->textEdit->copy();
    }

    void paste() {
        Tab *tab = getCurrentTab();
        if (tab)
            tab->textEdit->paste();
    }

    void showFind() {
        Tab *tab = getCurrentTab();
        if (!tab)
            return;

        if (findDialog) {
            findDialog->close();
            delete findDialog;
        }

        findDialog = new FindReplaceDialog(tab->textEdit, false, this);
        findDialog->show();
    }

    void showReplace() {
        Tab *tab = getCurrentTab();
        if (!tab)
            return;

        if (replaceDialog) {
            replaceDialog->close();
            delete replaceDialog;
        }

        replaceDialog = new FindReplaceDialog(tab->textEdit, true, this);
        replaceDialog->show();
    }

    void toggleTheme() {
        isDarkTheme = !isDarkTheme;
        applyTheme();
    }

    void showAbout() {
        QMessageBox::about(this, "About XamEditor",
                           "<h2>XamEditor 1.0</h2>"
                           "<p>A simple text editor with tabs and Gruvbox theming.</p>"
                           "<p>Copyright © 2025</p>"
                           "<p><a href='https://example.com'>https://example.com</a></p>");
    }

  private:
    void setupUI() {
        QWidget *centralWidget = new QWidget();
        setCentralWidget(centralWidget);

        QVBoxLayout *layout = new QVBoxLayout(centralWidget);
        layout->setContentsMargins(0, 0, 0, 0);

        tabWidget = new QTabWidget();
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);

        connect(tabWidget, &QTabWidget::tabCloseRequested, this, &XamEditor::closeTab);

        layout->addWidget(tabWidget);

        statusBar()->showMessage("Ready");
    }

    void setupMenus() {
        // File Menu
        QMenu *fileMenu = menuBar()->addMenu("&File");

        QAction *newAction = fileMenu->addAction("&New");
        newAction->setShortcut(QKeySequence::New);
        connect(newAction, &QAction::triggered, this, &XamEditor::newFile);

        QAction *openAction = fileMenu->addAction("&Open");
        openAction->setShortcut(QKeySequence::Open);
        connect(openAction, &QAction::triggered, this, &XamEditor::openFile);

        fileMenu->addSeparator();

        QAction *saveAction = fileMenu->addAction("&Save");
        saveAction->setShortcut(QKeySequence::Save);
        connect(saveAction, &QAction::triggered, this, &XamEditor::saveFile);

        QAction *saveAsAction = fileMenu->addAction("Save &As");
        saveAsAction->setShortcut(QKeySequence::SaveAs);
        connect(saveAsAction, &QAction::triggered, this, &XamEditor::saveFileAs);

        fileMenu->addSeparator();

        QAction *closeTabAction = fileMenu->addAction("&Close Tab");
        closeTabAction->setShortcut(QKeySequence("Ctrl+W"));
        connect(closeTabAction, &QAction::triggered, this, &XamEditor::closeTab);

        QAction *exitAction = fileMenu->addAction("E&xit");
        exitAction->setShortcut(QKeySequence::Quit);
        connect(exitAction, &QAction::triggered, this, &QWidget::close);

        // Edit Menu
        QMenu *editMenu = menuBar()->addMenu("&Edit");

        QAction *cutAction = editMenu->addAction("Cu&t");
        cutAction->setShortcut(QKeySequence::Cut);
        connect(cutAction, &QAction::triggered, this, &XamEditor::cut);

        QAction *copyAction = editMenu->addAction("&Copy");
        copyAction->setShortcut(QKeySequence::Copy);
        connect(copyAction, &QAction::triggered, this, &XamEditor::copy);

        QAction *pasteAction = editMenu->addAction("&Paste");
        pasteAction->setShortcut(QKeySequence::Paste);
        connect(pasteAction, &QAction::triggered, this, &XamEditor::paste);

        // Search Menu
        QMenu *searchMenu = menuBar()->addMenu("&Search");

        QAction *findAction = searchMenu->addAction("&Find");
        findAction->setShortcut(QKeySequence::Find);
        connect(findAction, &QAction::triggered, this, &XamEditor::showFind);

        QAction *replaceAction = searchMenu->addAction("&Replace");
        replaceAction->setShortcut(QKeySequence::Replace);
        connect(replaceAction, &QAction::triggered, this, &XamEditor::showReplace);

        // View Menu
        QMenu *viewMenu = menuBar()->addMenu("&View");

        QAction *themeAction = viewMenu->addAction("Toggle &Theme");
        themeAction->setShortcut(QKeySequence("Ctrl+T"));
        connect(themeAction, &QAction::triggered, this, &XamEditor::toggleTheme);

        // Help Menu
        QMenu *helpMenu = menuBar()->addMenu("&Help");

        QAction *aboutAction = helpMenu->addAction("&About");
        connect(aboutAction, &QAction::triggered, this, &XamEditor::showAbout);
    }

    void applyTheme() {
        QString stylesheet;

        if (isDarkTheme) {
            // Gruvbox Dark Theme
            stylesheet = R"(
                QMainWindow {
                    background-color: #282828;
                    color: #ebdbb2;
                }
                QMenuBar {
                    background-color: #3c3836;
                    color: #ebdbb2;
                    border-bottom: 1px solid #504945;
                }
                QMenuBar::item {
                    background-color: transparent;
                    padding: 4px 8px;
                }
                QMenuBar::item:selected {
                    background-color: #504945;
                }
                QMenu {
                    background-color: #3c3836;
                    color: #ebdbb2;
                    border: 1px solid #504945;
                }
                QMenu::item:selected {
                    background-color: #504945;
                }
                QTabWidget::pane {
                    border: 1px solid #504945;
                    background-color: #282828;
                }
                QTabBar::tab {
                    background-color: #3c3836;
                    color: #ebdbb2;
                    padding: 8px 12px;
                    margin-right: 2px;
                }
                QTabBar::tab:selected {
                    background-color: #504945;
                }
                QTextEdit {
                    background-color: #282828;
                    color: #ebdbb2;
                    border: none;
                    selection-background-color: #458588;
                }
                QStatusBar {
                    background-color: #3c3836;
                    color: #ebdbb2;
                }
                QDialog {
                    background-color: #282828;
                    color: #ebdbb2;
                }
                QLineEdit {
                    background-color: #3c3836;
                    color: #ebdbb2;
                    border: 1px solid #504945;
                    padding: 4px;
                }
                QPushButton {
                    background-color: #3c3836;
                    color: #ebdbb2;
                    border: 1px solid #504945;
                    padding: 6px 12px;
                }
                QPushButton:hover {
                    background-color: #504945;
                }
                QCheckBox {
                    color: #ebdbb2;
                }
            )";
        } else {
            // Gruvbox Light Theme
            stylesheet = R"(
                QMainWindow {
                    background-color: #fbf1c7;
                    color: #3c3836;
                }
                QMenuBar {
                    background-color: #ebdbb2;
                    color: #3c3836;
                    border-bottom: 1px solid #d5c4a1;
                }
                QMenuBar::item {
                    background-color: transparent;
                    padding: 4px 8px;
                }
                QMenuBar::item:selected {
                    background-color: #d5c4a1;
                }
                QMenu {
                    background-color: #ebdbb2;
                    color: #3c3836;
                    border: 1px solid #d5c4a1;
                }
                QMenu::item:selected {
                    background-color: #d5c4a1;
                }
                QTabWidget::pane {
                    border: 1px solid #d5c4a1;
                    background-color: #fbf1c7;
                }
                QTabBar::tab {
                    background-color: #ebdbb2;
                    color: #3c3836;
                    padding: 8px 12px;
                    margin-right: 2px;
                }
                QTabBar::tab:selected {
                    background-color: #d5c4a1;
                }
                QTextEdit {
                    background-color: #fbf1c7;
                    color: #3c3836;
                    border: none;
                    selection-background-color: #b57614;
                }
                QStatusBar {
                    background-color: #ebdbb2;
                    color: #3c3836;
                }
                QDialog {
                    background-color: #fbf1c7;
                    color: #3c3836;
                }
                QLineEdit {
                    background-color: #ebdbb2;
                    color: #3c3836;
                    border: 1px solid #d5c4a1;
                    padding: 4px;
                }
                QPushButton {
                    background-color: #ebdbb2;
                    color: #3c3836;
                    border: 1px solid #d5c4a1;
                    padding: 6px 12px;
                }
                QPushButton:hover {
                    background-color: #d5c4a1;
                }
                QCheckBox {
                    color: #3c3836;
                }
            )";
        }

        setStyleSheet(stylesheet);
    }

    Tab *getCurrentTab() {
        int index = tabWidget->currentIndex();
        if (index >= 0 && index < tabs.size()) {
            return tabs[index].get();
        }
        return nullptr;
    }

    void updateTabTitle(Tab *tab) {
        for (int i = 0; i < tabs.size(); ++i) {
            if (tabs[i].get() == tab) {
                QString title = QFileInfo(tab->filename).baseName();
                if (tab->isModified) {
                    title += " *";
                }
                tabWidget->setTabText(i, title);
                break;
            }
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    XamEditor editor;
    editor.show();

    return app.exec();
}

#include "xam.moc"
