#include "settingswindow.h"
#include "ui_settingswindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QDomDocument>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QPushButton>

SettingsWindow::SettingsWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    setWindowTitle("Settings");
    connect(ui->clearProgressButton, &QPushButton::clicked, this, &SettingsWindow::onClearProgressButtonClicked);

}




SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::setIgnoreCaseCheckBoxState(bool state) {
    ui->ignoreCaseCheckBox->setChecked(state);
};

void SettingsWindow::setIgnorePunctuationCheckBoxState(bool state) {
    ui->ignorePunctuationCheckBox->setChecked(state);
};

void SettingsWindow::setCurrentBookFileName(QString &fileName) {
    ui->BookFileName->setText(fileName);
    ui->clearProgressButton->setEnabled(ui->BookFileName->text() != "No book loaded");
};

QString processElement(QXmlStreamReader &xml) {

    QString text;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "p") {
                QString pText;
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "p") && !xml.atEnd()) {
                    if (xml.tokenType() == QXmlStreamReader::Characters) {
                        pText += xml.text().toString();
                    }
                    xml.readNext();
                }
                text += pText + " ";
            }
            else {
                text += processElement(xml);
            }
        }
        else if (xml.tokenType() == QXmlStreamReader::EndElement) {
            if (xml.name() == "section" || xml.name() == "body")
                break;
        }
    }
    return text;
}
QRegularExpression nExpression("\\n+");

QString replaceSomeSymbolsIn(QString &text) {

    text.replace("«", "\"");
    text.replace("»", "\"");

    text.replace("—", "-");
    text.replace("–", "-");

    text.replace("’", "`");

    text.replace(QChar(0x00A0), " "); // "No-break space"
    text.replace(nExpression, " ");
    text.remove("  ");

    return text;
};

QString extractFb2Text(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << fileName;
        return "";
    }

    QXmlStreamReader xml(&file);
    QString extractedText;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "body") {
            extractedText += processElement(xml);
        }
    }
    if (xml.hasError()) {
        qWarning() << "XML Error:" << xml.errorString();
    }
    file.close();
    replaceSomeSymbolsIn(extractedText);

    return extractedText;
}

void SettingsWindow::on_OpenBookFileButton_pressed() {

    QString filePath = QFileDialog::getOpenFileName(this, "Select a book file", "", "FB2 books (*.fb2*)");
    if (filePath == "") return;
    textFromFile = extractFb2Text(filePath);
    emit setCurrentBookFilePath(filePath);
    ui->BookFileName->setText(filePath);
    ui->clearProgressButton->setEnabled(true);
    emit fileTextSent(textFromFile);
}



void SettingsWindow::on_ignoreCaseCheckBox_checkStateChanged(const Qt::CheckState &state)
{
    emit setIgnoreCaseState(state);
}

void SettingsWindow::on_ignorePunctuationCheckBox_checkStateChanged(const Qt::CheckState &state)
{
    emit setIgnorePunctuationState(state);
}


void SettingsWindow::onClearProgressButtonClicked()
{
    emit clearProgressOfBook();
    textFromFile = extractFb2Text(ui->BookFileName->text());
    ui->clearProgressButton->setEnabled(false);
    emit fileTextSent(textFromFile);
}

