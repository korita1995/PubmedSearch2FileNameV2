#ifndef MAINDIALOG_H
#define MAINDIALOG_H
#include "ui_widget.h"
#include <QDialog>

// classの前方参照
class QLabel;
class QPushButton;
class QPlainTextEdit;
class QNetworkAccessManager;
class QNetworkReply;

class mainDialog : public QDialog, public Ui::Widget{
    Q_OBJECT

public:
    mainDialog(QWidget *parent=nullptr); // 親を持たないウィジェットであると宣言
    void searchPubmedKeyword();
    QPlainTextEdit *keywordTextEdit;
    QPlainTextEdit *resultTextEdit;

signals:
   void statusMessage(QString);

public slots:
    void searchPubmed();
    void keywordSearchFinished(QNetworkReply*);
    void idSearchFinished(QNetworkReply*);
    void exactSearchPubmedKeyword();
    void exactSearchFinished(QNetworkReply*);
    void searchCrossref();
    void keywordSearchCrossrefFinished(QNetworkReply*);
    void doiSearchCrossrefFinished(QNetworkReply*);
    void arxivSearchFinished(QNetworkReply*);
    void clipSearch();
    void updateClipboard();
    void enableButtons();
    void enableClipButton();
    void searchArxiv();

private:
    QLabel *keywordLabel;
    QLabel *resultLabel;
    QPushButton *keywordSearchButton;
    QPushButton *clipSearchButton;
    QPushButton *keywordExactSearchButton;
    QPushButton *keywordCrossrefSearchButton;
    QPushButton *keywordArxivSearchButton;
    QNetworkAccessManager *keywordSearchManager;
    QNetworkAccessManager *idSearchManager;
    QNetworkAccessManager *keywordExactSearchManager1;
    QNetworkAccessManager *keywordExactSearchManager2;
    QNetworkAccessManager *keywordCrossrefSearchManager;
    QNetworkAccessManager *doiCrossrefSearchManager;
    QNetworkAccessManager *keywordArxivSearchManager;
    QString authorNameStrip(QString);
    QString authorNameStripArxiv(QString);
    QString createNewFileName(QJsonObject);
    bool checkAllNumber(QString);
    bool checkDoi(QString);
    void searchPubmedId();
    void keywordSearchCrossref();
    void doiSearchCrossref();
    QString uid;
    QString esearchBaseUrl;
    QString esummaryBaseUrl;
    QString keywordCrossrefBaseUrl;
    QString doiCrossrefBaseUrl;
    QString arxivBaseUrl;
    QString extractPaperTitle(QJsonObject);
    int levenshteinDistance(QString, QString);
    QString extractFromCrossrefJson(QJsonObject);
    QClipboard *clipboard;
    void enableAllButtons();
    void disableAllButtons();
};

#endif // MAINDIALOG_H
