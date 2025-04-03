#include "typingwindow.h"
#include "ui_typingwindow.h"
#include <QDebug>
#include <QChar>
#include <QString>
#include <QClipboard>
#include <QGuiApplication>
#include <QRegularExpression>
#include <vector>
#include <QFont>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

void initializeJsonFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.exists()) {

        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QJsonObject jsonObject;
            jsonObject["ignoreCaseCheckBoxState"] = false;
            jsonObject["ignorePunctuationCheckBoxState"] = false;
            jsonObject["books"] = QJsonObject();

            QJsonDocument jsonDoc(jsonObject);
            file.write(jsonDoc.toJson());
            file.close();

        } else {
            qWarning() << "Ошибка создания файла:" << file.errorString();
        }
    }
}


void TypingWindow::initFromJsonFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл:" << filePath;
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if (document.isNull()) {
        qWarning() << "Ошибка парсинга JSON.";
        return;
    }

    QJsonObject jsonObject = document.object();
    this->ignoreCase = jsonObject["ignoreCaseCheckBoxState"].toBool();
    this->ignorePunctuation = jsonObject["ignorePunctuationCheckBoxState"].toBool();
    this->booksObject = jsonObject["books"].toObject();
}

void TypingWindow::incremCurrentBookProgress() {
    QFile file(this->settingsFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if (document.isNull()) {
        qWarning() << "Ошибка парсинга JSON.";
        return;
    }

    QJsonObject jsonObject = document.object();
    QJsonObject booksObject = jsonObject["books"].toObject();

    booksObject[this->currentBookFilePath] = booksObject[this->currentBookFilePath].toInt() + 1;
    jsonObject["books"] = booksObject;

    QFile writeFile(this->settingsFilePath);
    if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    writeFile.write(QJsonDocument(jsonObject).toJson());
    writeFile.close();
};


void TypingWindow::updateCheckBoxStateInJsonFile(const QString CheckBoxName, bool State) {
    QFile file(this->settingsFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if (document.isNull()) {
        qWarning() << "Ошибка парсинга JSON.";
        return;
    }

    QJsonObject jsonObject = document.object();
    jsonObject[CheckBoxName] = State;

    QFile writeFile(this->settingsFilePath);
    if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    writeFile.write(QJsonDocument(jsonObject).toJson());
    writeFile.close();
}

TypingWindow::TypingWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TypingWindow)
{
    ui->setupUi(this);
    QString filePath = "settings.json";
    initializeJsonFile(filePath);
    initFromJsonFile(filePath);
}

TypingWindow::~TypingWindow()
{
    delete ui;
}


class Letter {
private:
    QString BLACK_COLOR = "#020202";
    QString GREY_COLOR = "#959595";
    QString RED_COLOR = "#D84040";
    QChar letter;
    bool isPressed;
    bool isWrong;
    bool isCurrent;

public:
    Letter(QChar i_letter): letter(i_letter), isPressed(false), isWrong(false), isCurrent(false) {};
    Letter(QChar i_letter, bool i_isPressed): letter(i_letter), isPressed(i_isPressed), isWrong(false), isCurrent(false) {};

    QString formatToHTML() {
        QString formatedLetter = "<span style=\"color:"; /* start html code for letter */
                formatedLetter += (this->isPressed ? this->GREY_COLOR : (this->isWrong ? this->RED_COLOR : this->BLACK_COLOR)) + ";\">"; /* What color is letter */
                formatedLetter += (isCurrent ? "<u>" : "");
                formatedLetter += this->letter;        /* Adding letter, if space -> underline */
                formatedLetter += (isCurrent ? "</u>" : "");
                formatedLetter += "</span>"; /* end html code for letter */
        return formatedLetter;
    };

    void setPressed(bool State) {
        this->isPressed = State;
    };

    void setWrong(bool State) {
        this->isWrong = State;
    };

    bool getPressedState() {
        return this->isPressed;
    };

    void setCurrent(bool State) {
        this->isCurrent = State;
    };

    QChar getChar(bool isIgnoreCase) {
        if (isIgnoreCase) {
            return this->letter.toLower();
        }
        return this->letter;
    };
};




class TextLabel {
private:
    QLabel* labelPointer;
    std::vector<Letter> currentTextObjects;
    bool isFinished = false;

public:
    TextLabel(QLabel* pointerToLabel): labelPointer(pointerToLabel) {};

    void render(std::vector<Letter> &textObjects) {
        QString textToSet = "";
        for (Letter i : textObjects) {
            textToSet += i.formatToHTML();
            this->currentTextObjects.push_back(i);
        };
        this->labelPointer->setText(textToSet);
    };

    void render() {
        QString textToSet = "";
        for (Letter i : this->currentTextObjects) {
            textToSet += i.formatToHTML();
        };
        this->labelPointer->setText(textToSet);
    };


    Letter* GetFirstNotPressed() {
        Letter* letter = nullptr;
        for(int i = 0; i<(int)this->currentTextObjects.size(); i++) {
            Letter* letterObj = &this->currentTextObjects[i];
            if (letterObj->getPressedState()) continue;
            letterObj->setCurrent(true);
            letter = letterObj;
            break;
        };
        return letter;
    };

    void setFinishedState(bool State) {
        this->isFinished = State;
    };

    bool getFinishedState() {
        return this->isFinished;
    };
};


std::vector<Letter> allTextLetters;
std::vector<TextLabel*> labelsVect;

void skipNLetters(std::vector<Letter> &LettersVector, int N) {
    while (N > 0 && LettersVector[N].getChar(false) != ' ') --N;
    if (LettersVector[N].getChar(false) == ' ') ++N;

    LettersVector.erase(LettersVector.begin(), LettersVector.begin() + N);
}

int TypingWindow::loadBookProgressFromJson() {
    if (this->currentBookFilePath == "No book loaded") {
        return 0;
    };

    QFile file(this->settingsFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 0;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if (document.isNull()) {
        qWarning() << "Ошибка парсинга JSON.";
        return 0;
    }

    QJsonObject jsonObject = document.object();
    QJsonObject booksObject = jsonObject["books"].toObject();

    if (booksObject.contains(this->currentBookFilePath)) {
        return booksObject[this->currentBookFilePath].toInt();

    } else {
        booksObject[this->currentBookFilePath] = 0;
        jsonObject["books"] = booksObject;

        QFile writeFile(this->settingsFilePath);
        if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return 0;
        }
        writeFile.write(QJsonDocument(jsonObject).toJson());
        writeFile.close();
        return 0;
    };
}

std::vector<Letter> TextToLettersObj(QString textToConvert) {
    std::vector<Letter> lettersVect;
    for (QChar l : textToConvert) {
        lettersVect.push_back(Letter(l));
    };
    return lettersVect;
}

std::vector<Letter> getFirstNFromAll(int N) {
    // Ограничиваем N размером вектора
    N = std::min(N, static_cast<int>(allTextLetters.size()));
    if (N == 0)
        return {};

    int cutIndex = N - 1;

    if (allTextLetters[N - 1].getChar(false) != ' ' && N != (int)allTextLetters.size()) {

        for (; cutIndex >= 0; --cutIndex) {
            if (allTextLetters[cutIndex].getChar(false) == ' ')
                break;
        }
    }



    if (cutIndex < 0)
        return {};

    int count = cutIndex + 1;
    std::vector<Letter> firstN;
    firstN.reserve(count);
    std::move(allTextLetters.begin(), allTextLetters.begin() + count, std::back_inserter(firstN));
    allTextLetters.erase(allTextLetters.begin(), allTextLetters.begin() + count);

    return firstN;
}


void TypingWindow::initText(QString &text) {
    allTextLetters = TextToLettersObj(text);
    skipNLetters(allTextLetters, loadBookProgressFromJson());
    allTextLetters[0].setCurrent(true);
    initText();
};

void TypingWindow::initText() {
    labelsVect = {new TextLabel(ui->textLabel1), new TextLabel(ui->textLabel2), new TextLabel(ui->textLabel3)};

    for (int i = 0; i<(int)labelsVect.size(); i++) {
        TextLabel* label = labelsVect[i];
        std::vector<Letter> FirstNFromAll = getFirstNFromAll(50);
        label->render(FirstNFromAll);
    };
};

void TypingWindow::handleTextFromFile(QString &text) {
    initText(text);
}

void TypingWindow::setIgnoreCaseState(Qt::CheckState state){
    this->ignoreCase = state;
    updateCheckBoxStateInJsonFile("ignoreCaseCheckBoxState", state);
};

void TypingWindow::setIgnorePunctuationState(Qt::CheckState state){
    this->ignorePunctuation = state;
    updateCheckBoxStateInJsonFile("ignorePunctuationCheckBoxState", state);
};

void TypingWindow::setCurrentBookFilePath(QString &filePath) {
    this->currentBookFilePath = filePath;
};

void TypingWindow::clearProgressOfBook() {
    QFile file(this->settingsFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if (document.isNull()) {
        return;
    }

    QJsonObject jsonObject = document.object();
    QJsonObject booksObject = jsonObject["books"].toObject();

    if (booksObject.contains(this->currentBookFilePath)) {
        booksObject.remove(this->currentBookFilePath);
        jsonObject["books"] = booksObject;

        QFile writeFile(this->settingsFilePath);
        if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return;
        }

        writeFile.write(QJsonDocument(jsonObject).toJson());
        writeFile.close();
    } else {
        return;
    }
};

bool containsAny(const QString &str, const QStringList &symbols) {
    for (const auto &symbol : symbols) {
        if (str.contains(symbol)) {
            return true;
        }
    }
    return false;
}
QRegularExpression nExpressionn("\\n+");

QString TypingWindow::replaceSomeSymbolsIn(QString &text) {

    text.replace("«", "\"");
    text.replace("»", "\"");

    text.replace("—", "-");
    text.replace("–", "-");

    text.replace("’", "`");

    text.remove(QChar(0x00A0)); // "No-break space"
    text.replace(nExpressionn, " ");
    text.remove("  ");

    return text;
};

void TypingWindow::keyPressEvent(QKeyEvent *event) {
    if (event->text() == "\u000F") { // ctrl + O
        qDebug() << "Нажат шорткат налаштувань!";

        settingsWindow = new SettingsWindow();

        settingsWindow->setIgnoreCaseCheckBoxState(this->ignoreCase);
        settingsWindow->setIgnorePunctuationCheckBoxState(this->ignorePunctuation);
        settingsWindow->setCurrentBookFileName(this->currentBookFilePath);


        connect(settingsWindow, &SettingsWindow::clearProgressOfBook, this, &TypingWindow::clearProgressOfBook);
        connect(settingsWindow, &SettingsWindow::setCurrentBookFilePath, this, &TypingWindow::setCurrentBookFilePath);
        connect(settingsWindow, &SettingsWindow::fileTextSent, this, &TypingWindow::handleTextFromFile);
        connect(settingsWindow, &SettingsWindow::setIgnoreCaseState, this, &TypingWindow::setIgnoreCaseState);
        connect(settingsWindow, &SettingsWindow::setIgnorePunctuationState, this, &TypingWindow::setIgnorePunctuationState);

        settingsWindow->show();

    } else if (event->text() == "\u0016") { // ctrl + V
        QClipboard *clipboard = QGuiApplication::clipboard();

        QString text = clipboard->text();
        replaceSomeSymbolsIn(text);
        qDebug() << text;
        initText(text);
    } else {

        if (event->text() != "") {

            for (int i = 0; i<(int)labelsVect.size(); i++) {
                TextLabel* label = labelsVect[i];

                if (label->getFinishedState() && i != (int)labelsVect.size()-1) continue;


                Letter* currentFirstNotPressed = label->GetFirstNotPressed();
                if (currentFirstNotPressed == nullptr) {
                    label->setFinishedState(true);
                    continue;
                };


                if (currentFirstNotPressed->getChar(ignoreCase) == event->text() || event->text() == "\u001A") { /* "\u001A" (CTRL + Z) SKIP LETTERS */

                    if (this->currentBookFilePath != "No loaded book") incremCurrentBookProgress();
                    currentFirstNotPressed->setCurrent(false);
                    currentFirstNotPressed->setPressed(true);

                    if (label->GetFirstNotPressed() == nullptr && i != (int)labelsVect.size()-1) {
                        labelsVect[i+1]->GetFirstNotPressed()->setCurrent(true);
                        labelsVect[i+1]->render();
                    };


                    if (ignorePunctuation) {
                        while (label->GetFirstNotPressed() != nullptr && punctuationToIgnore.contains(label->GetFirstNotPressed()->getChar(ignoreCase))) {
                            Letter* punctuationSymbol = label->GetFirstNotPressed();
                            punctuationSymbol->setCurrent(false);
                            punctuationSymbol->setPressed(true);
                        };
                    };


                    if (label->GetFirstNotPressed() == nullptr && i == (int)labelsVect.size()-1) {
                        initText();
                        break;
                    };
                } else {
                    currentFirstNotPressed->setWrong(true);
                };
                label->render();
                break;
            };
        };
    };
};










