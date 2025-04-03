#ifndef TYPINGWINDOW_H
#define TYPINGWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QString>
#include "settingswindow.h"
#include <QJsonObject>
QT_BEGIN_NAMESPACE
namespace Ui {
class TypingWindow;
}
QT_END_NAMESPACE

class TypingWindow : public QMainWindow
{
    Q_OBJECT

public:
    TypingWindow(QWidget *parent = nullptr);
    ~TypingWindow();

protected:
    void initFromJsonFile(const QString& filePath);
    void updateCheckBoxStateInJsonFile(const QString CheckBoxName, bool State);
    QJsonObject booksObject;
    void addBookToJson();
    int loadBookProgressFromJson();
    void incremCurrentBookProgress();
    void clearProgressOfBook();

    void keyPressEvent(QKeyEvent *event) override;
    void handleTextFromFile(QString &text);
    void initText(QString &text);
    void initText();

    bool ignoreCase = false;
    void setIgnoreCaseState(Qt::CheckState state);

    bool ignorePunctuation = false;
    void setIgnorePunctuationState(Qt::CheckState state);

    void setCurrentBookFilePath(QString &filePath);


    QString punctuationToIgnore = ".,;:@#$%^&*()_-=+/\"`!?";
    QString replaceSomeSymbolsIn(QString &text);


private:
    QString currentBookFilePath = "No book loaded";
    const QString settingsFilePath= "settings.json";
    Ui::TypingWindow *ui;
    SettingsWindow *settingsWindow;
};
#endif // TYPINGWINDOW_H
