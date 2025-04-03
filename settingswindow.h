#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include <QLabel>
namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

    void setIgnoreCaseCheckBoxState(bool state);
    void setIgnorePunctuationCheckBoxState(bool state);
    void setCurrentBookFileName(QString& fileName);

signals:

    void setCurrentBookFilePath(QString& filePath);
    void clearProgressOfBook();
    void fileTextSent(QString& text);
    void setIgnoreCaseState(const Qt::CheckState &arg1);
    void setIgnorePunctuationState(const Qt::CheckState &arg1);


private slots:
    void on_OpenBookFileButton_pressed();
    void on_ignoreCaseCheckBox_checkStateChanged(const Qt::CheckState &state);
    void on_ignorePunctuationCheckBox_checkStateChanged(const Qt::CheckState &state);

    void onClearProgressButtonClicked();

protected:
    Ui::SettingsWindow *ui;
private:

    QString textFromFile;
};

#endif // SETTINGSWINDOW_H
