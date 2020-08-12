#include "mainDialog.h"
#include "ui_widget.h"
#include "widget.h"
#include <QtWidgets>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QApplication>
#include <QEventLoop>
#include <QDomDocument>

//　コンストラクタ
mainDialog::mainDialog(QWidget *parent)
    : QDialog(parent){
    // .uiファイルから設定の読み込み
    setupUi(this);

    // ウィジェットの初期化
    keywordLabel = label;
    resultLabel = label_2;
    keywordSearchButton = pushButton_3;
    clipSearchButton = pushButton_4;
    keywordExactSearchButton = pushButton;
    keywordCrossrefSearchButton = pushButton_2;
    keywordArxivSearchButton = pushButton_5;
    keywordTextEdit = plainTextEdit;
    resultTextEdit = plainTextEdit_2;
    uid = tr("");
    keywordSearchManager = new QNetworkAccessManager;
    idSearchManager = new QNetworkAccessManager;
    keywordExactSearchManager1 = new QNetworkAccessManager;
    keywordExactSearchManager2 = new QNetworkAccessManager;
    keywordCrossrefSearchManager = new QNetworkAccessManager;
    doiCrossrefSearchManager = new QNetworkAccessManager;
    keywordArxivSearchManager = new QNetworkAccessManager;
    clipboard = QApplication::clipboard();

    // 検索URLの設定
    esearchBaseUrl= tr("https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esearch.fcgi?db=pubmed&sort=relevance&retmode=json&term=");
    esummaryBaseUrl= tr("https://eutils.ncbi.nlm.nih.gov/entrez/eutils/esummary.fcgi?db=pubmed&retmode=json&id=");
    keywordCrossrefBaseUrl = tr("https://api.crossref.org/works?sort=relevance&query=");
    doiCrossrefBaseUrl = tr("https://api.crossref.org/works/");
    arxivBaseUrl= tr("http://export.arxiv.org/api/query?&sortBy=relevance&sortOrder=ascending&max_results=10&search_query=%22");

    // ボタンの無効化
    keywordSearchButton->setEnabled(false);
    keywordExactSearchButton->setEnabled(false);
    keywordCrossrefSearchButton->setEnabled(false);
    keywordArxivSearchButton->setEnabled(false);

    // テキストエディタに対するドラッグ＆ドロップの無効化
    resultTextEdit->setAcceptDrops(false);
    keywordTextEdit->setAcceptDrops(false);

    // シグナルとスロットの接続
    connect(keywordSearchButton, SIGNAL(clicked()), this, SLOT(searchPubmed()));
    connect(keywordSearchManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(keywordSearchFinished(QNetworkReply*)));
    connect(idSearchManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(idSearchFinished(QNetworkReply*)));
    connect(keywordExactSearchButton, SIGNAL(clicked()), this, SLOT(exactSearchPubmedKeyword()));
    connect(keywordExactSearchManager1, SIGNAL(finished(QNetworkReply*)), this, SLOT(exactSearchFinished(QNetworkReply*)));
    connect(keywordCrossrefSearchButton, SIGNAL(clicked()), this, SLOT(searchCrossref()));
    connect(keywordCrossrefSearchManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(keywordSearchCrossrefFinished(QNetworkReply*)));
    connect(doiCrossrefSearchManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(doiSearchCrossrefFinished(QNetworkReply*)));
    connect(clipSearchButton, SIGNAL(clicked()), this, SLOT(clipSearch()));
    connect(resultTextEdit, SIGNAL(textChanged()), this, SLOT(updateClipboard()));
    connect(keywordTextEdit, SIGNAL(textChanged()), this, SLOT(enableButtons()));
    connect(clipboard, SIGNAL(dataChanged()), this, SLOT(enableClipButton()));
    connect(keywordArxivSearchButton, SIGNAL(clicked()), this, SLOT(searchArxiv()));
    connect(keywordArxivSearchManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(arxivSearchFinished(QNetworkReply*)));

    // 起動時にPDFをドラッグ＆ドロップ or クリップボードにデータが存在する場合にPubmed検索
    QStringList argv = QCoreApplication::arguments();
    // コマンドライン引数が与えられなかった場合、クリップボードから検索を実行
    if (argv.size() == 1){
        QString text = clipboard->text();
        text.remove(" ");
        text.remove("\t");
        text.remove("\n");

        // クリップボードにデータが存在しない場合
        if (text.isEmpty()){
            clipSearchButton->setEnabled(false);
        }
        // クリップボードにデータが存在する場合
        else{
            clipSearch();
        }
    }
    // 起動時にPDFをドラッグ＆ドロップしたとき、ファイル名をもとにPubmed検索
    else if (argv.size() == 2){
        argv.removeAt(0); // コマンドライン引数のうち、プログラム自身を除く
        // PDFファイル以外の場合は検索を実行しない
        if (argv[0].contains(tr(".pdf")) || argv[0].contains(tr(".PDF"))){
            QString fileName = QFileInfo(argv[0]).baseName();
            keywordTextEdit->setPlainText(fileName);
            resultTextEdit->clear();
            searchPubmed();
        }
    }
}

//----Pubmed検索関連の関数----
// 「クリップボードから検索」ボタンが押されたときにPubmed検索を実行
void mainDialog::clipSearch(){
    QString text = clipboard->text();
    keywordTextEdit->setPlainText(text);
    resultTextEdit->clear();
    if (checkAllNumber(text)){
        uid = text;
        searchPubmedId();
    }
    else{
        searchPubmedKeyword();
    }
}

// 「入力欄から検索」ボタンが押されたときにPubmed検索を実行
void mainDialog::searchPubmed(){
    QString input = keywordTextEdit->toPlainText();
    resultTextEdit->clear();
    if (checkAllNumber(input)){
        uid = input;
        searchPubmedId();
    }
    else{
        searchPubmedKeyword();
    }
}

// Pubmedでキーワード検索
void mainDialog::searchPubmedKeyword(){
    //resultTextEdit->setPlainText(QString::fromLocal8Bit("Pubmedでキーワード検索を実行開始"));
    statusMessage(QString::fromLocal8Bit("Pubmedでキーワード検索を実行開始"));
    disableAllButtons();
    QString keyword = keywordTextEdit->toPlainText();
    if (keyword.contains(tr("https://doi.org/"))){
        keyword.remove(tr("https://doi.org/"));
    }
    else if (keyword.contains(tr("doi: "))){
        keyword.remove(tr("doi: "));
        if (keyword.right(1) == tr(".")) {
            keyword.chop(1);
        }
    }
    else if (keyword.contains(tr("DOI: "))) {
        keyword.remove(tr("DOI: "));
        if (keyword.right(1) == tr(".")) {
            keyword.chop(1);
        }
    }
    if (checkDoi(keyword)){
        QString esearchUrl = esearchBaseUrl + keyword;
        keywordSearchManager->get(QNetworkRequest(QUrl(esearchUrl)));
    }
    else{
        keyword.replace(QString(" "), QString("+"));
        keyword.replace(QString("/"), QString(""));
        QString esearchUrl = esearchBaseUrl + keyword;
        keywordSearchManager->get(QNetworkRequest(QUrl(esearchUrl)));
    }
}

// searchPubmedKeyword()の終了時に実行
void mainDialog::keywordSearchFinished(QNetworkReply* reply){
    if(reply->error() == QNetworkReply::NoError){
        QString esearchResultStr = QString::fromUtf8(reply->readAll().data());
        QJsonDocument jsonDoc = QJsonDocument::fromJson(esearchResultStr.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();
        uid = jsonObj["esearchresult"].toObject()["idlist"].toArray()[0].toString();
        if (uid == tr("")){
            //resultTextEdit->setPlainText(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
            statusMessage(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
            clipSearchButton->setEnabled(false);
            //enableAllButtons();
            searchCrossref();
        }
        else{
            searchPubmedId();
        }
    }
    else{
        //resultTextEdit->setPlainText(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        statusMessage(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        enableAllButtons();
    }
}

// PubmedでPMID検索
void mainDialog::searchPubmedId(){
    //resultTextEdit->setPlainText(QString::fromLocal8Bit("PubmedでPMID検索を実行開始"));
    statusMessage(QString::fromLocal8Bit("PubmedでPMID検索を実行開始"));
    disableAllButtons();
    QString esummaryUrl = esummaryBaseUrl + uid;
    idSearchManager->get(QNetworkRequest(QUrl(esummaryUrl)));
}

// searchPubmedId()の終了時に実行
void mainDialog::idSearchFinished(QNetworkReply* reply){
    if(reply->error() == QNetworkReply::NoError){
        QString esummaryResultStr = QString::fromUtf8(reply->readAll().data());
        QJsonDocument jsonDoc = QJsonDocument::fromJson(esummaryResultStr.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();
        QJsonObject jsonSubObj = jsonObj["result"].toObject()[uid].toObject();
        QString newFileName = createNewFileName(jsonSubObj);
        statusMessage(QString::fromLocal8Bit(""));
        clipboard->setText(newFileName);
        resultTextEdit->setPlainText(newFileName);
        enableAllButtons();
    }
    else{
        //resultTextEdit->setPlainText(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        statusMessage(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        enableAllButtons();
    }
}

QString mainDialog::authorNameStrip(QString fullName) {
    int num = fullName.split(" ").size();
    //QString familyName = fullName.remove(tr(" ") + fullName.split(" ")[num-1]);
    QString familyName = tr("");
    for (int i=0; i<num-1;++i){
        familyName += fullName.split(" ")[i];
        familyName += tr(" ");
    }
    familyName.chop(1);
    return familyName;
}

// Pubmed検索結果がまとめられたJsonオブジェクトから論文情報抽出
QString mainDialog::createNewFileName(QJsonObject j){
    QString firstAuthorFullName = j["authors"].toArray()[0].toObject()["name"].toString();
    //QString firstAuthorFamilyName = firstAuthorFullName.split(" ")[0];
    QString firstAuthorFamilyName = authorNameStrip(firstAuthorFullName);

    QString lastAuthorFullName = j["lastauthor"].toString();
    //QString lastAuthorFamilyName = lastAuthorFullName.split(" ")[0];
    QString lastAuthorFamilyName = authorNameStrip(lastAuthorFullName);

    QString authors;
    if (j["authors"].toArray().size() == 1){
        authors = firstAuthorFamilyName;
    }
    else{
        authors = firstAuthorFamilyName + tr(" ") + lastAuthorFamilyName;
    }

    QString paperTitle = j["title"].toString();
    if (paperTitle.right(1) == tr(".")){
        paperTitle.chop(1);
    }
    paperTitle.remove(QRegExp("[/:*\?\"\'<>|]"));

    QString journalName = j["source"].toString();

    QString pubDate = j["pubdate"].toString();
    QString pubYear = pubDate.left(4);

    return authors + tr(" (") + journalName + tr(" ") + pubYear + tr(") ") + paperTitle;
}

// 「入力欄から高精度検索」ボタンが押されたときにPubmed検索を実行
void mainDialog::exactSearchPubmedKeyword(){
    QString keyword = keywordTextEdit->toPlainText();
    if (keyword.contains(tr("https://doi.org/"))){
        keyword.remove(tr("https://doi.org/"));
    }
    else if (keyword.contains(tr("doi: "))){
        keyword.remove(tr("doi: "));
        if (keyword.right(1) == tr(".")) {
            keyword.chop(1);
        }
    }
    else if (keyword.contains(tr("DOI: "))) {
        keyword.remove(tr("DOI: "));
        if (keyword.right(1) == tr(".")) {
            keyword.chop(1);
        }
    }
    resultTextEdit->clear();
    //resultTextEdit->setPlainText(QString::fromLocal8Bit("Pubmedでキーワード高精度検索を実行開始"));
    statusMessage(QString::fromLocal8Bit("Pubmedでキーワード高精度検索を実行開始"));
    disableAllButtons();
    if (checkDoi(keyword)){
        QString esearchUrl = esearchBaseUrl + keyword;
        keywordExactSearchManager1->get(QNetworkRequest(QUrl(esearchUrl)));
    }
    else{
        keyword.replace(QString(" "), QString("+"));
        keyword.replace(QString("/"), QString(""));
        QString esearchUrl = esearchBaseUrl + keyword;
        keywordExactSearchManager1->get(QNetworkRequest(QUrl(esearchUrl)));
    }
}

// exactSearchPubmedKeyword()の終了時に実行
void mainDialog::exactSearchFinished(QNetworkReply* reply){
    QString probNewFileName;
    int lowestDist = 1000;
    if(reply->error() == QNetworkReply::NoError){
        QString esearchResultStr = QString::fromUtf8(reply->readAll().data());
        QJsonDocument jsonDoc = QJsonDocument::fromJson(esearchResultStr.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();
        QJsonArray uids = jsonObj["esearchresult"].toObject()["idlist"].toArray();
        int uidsNum = uids.size();
        QEventLoop loop;
        for (int i =0; i<uidsNum; ++i){
            //resultTextEdit->setPlainText(QString::fromLocal8Bit("高精度検索を実行中: ") + QString::number(i+1) + tr("/") + QString::number(uidsNum));
            statusMessage(QString::fromLocal8Bit("高精度検索を実行中: ") + QString::number(i+1) + tr("/") + QString::number(uidsNum));
            clipSearchButton->setEnabled(false);
            uid = uids[i].toString();
            QString esummaryUrl = esummaryBaseUrl + uid;
            QNetworkReply *reply2 = keywordExactSearchManager2->get(QNetworkRequest(QUrl(esummaryUrl)));
            connect(keywordExactSearchManager2, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
            loop.exec();
            QString esummaryResultStr = QString::fromUtf8(reply2->readAll().data());
            QJsonDocument jsonDoc2 = QJsonDocument::fromJson(esummaryResultStr.toUtf8());
            QJsonObject jsonObj2 = jsonDoc2.object();
            QJsonObject jsonObj2Sub = jsonObj2["result"].toObject()[uid].toObject();
            QString paperTitle = extractPaperTitle(jsonObj2Sub);
            QString newFileName = createNewFileName(jsonObj2Sub);
            QString keyword = keywordTextEdit->toPlainText();
            int lvDist = levenshteinDistance(keyword, paperTitle);
            if (lvDist < lowestDist) {
                lowestDist = lvDist;
                probNewFileName = newFileName;
            }
            keywordExactSearchManager2->clearAccessCache();
        }
        if (lowestDist == 1000){
            //resultTextEdit->setPlainText(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
            statusMessage(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
            clipSearchButton->setEnabled(false);
            //enableAllButtons();
            searchCrossref();
        }
        else{
            statusMessage(QString::fromLocal8Bit(""));
            clipboard->setText(probNewFileName);
            resultTextEdit->setPlainText(probNewFileName);
            enableAllButtons();
        }
    }
    else{
        //resultTextEdit->setPlainText(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        statusMessage(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        clipSearchButton->setEnabled(false);
        //enableAllButtons();
        searchCrossref();
    }
}
//----End----

//----Crossref検索関連の関数----
// 「Crossref検索」ボタンが押されたときにCrossref検索を実行
void mainDialog::searchCrossref(){
    QString keyword = keywordTextEdit->toPlainText();
    if (keyword.contains(tr("https://doi.org/"))){
        keyword.remove(tr("https://doi.org/"));
    }
    else if (keyword.contains(tr("doi: "))){
        keyword.remove(tr("doi: "));
        if (keyword.right(1) == tr(".")) {
            keyword.chop(1);
        }
    }
    else if (keyword.contains(tr("DOI: "))) {
        keyword.remove(tr("DOI: "));
        if (keyword.right(1) == tr(".")) {
            keyword.chop(1);
        }
    }
    resultTextEdit->clear();
    if (checkDoi(keyword)){
        doiSearchCrossref();
    }
    else{
        keywordSearchCrossref();
    }
}

// Crossrefでキーワード検索
void mainDialog::keywordSearchCrossref(){
    QString keyword = keywordTextEdit->toPlainText();
    //resultTextEdit->setPlainText(QString::fromLocal8Bit("Crossrefでキーワード検索を実行開始"));
    statusMessage(QString::fromLocal8Bit("Crossrefでキーワード検索を実行開始"));
    disableAllButtons();
    keyword.replace(QString(" "), QString("+"));
    QString url = keywordCrossrefBaseUrl + keyword;
    keywordCrossrefSearchManager->get(QNetworkRequest(QUrl(url)));
}

// keywordSearchCrossref()の終了時に実行
void mainDialog::keywordSearchCrossrefFinished(QNetworkReply* reply){
    if(reply->error() == QNetworkReply::NoError){
        QString searchResultStr = QString::fromUtf8(reply->readAll().data());
        QJsonDocument jsonDoc = QJsonDocument::fromJson(searchResultStr.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();
        QJsonObject jsonObjSub = jsonObj["message"].toObject()["items"].toArray()[0].toObject();
        QString newFileName = extractFromCrossrefJson(jsonObjSub);
        if (newFileName == tr("  ( 0) ") || newFileName == tr("")){
            //resultTextEdit->setPlainText(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
            statusMessage(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
            enableAllButtons();
        }
        else{
            clipboard->setText(newFileName);
            resultTextEdit->setPlainText(newFileName);
            enableAllButtons();
        }
    }
    else{
        //resultTextEdit->setPlainText(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        statusMessage(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        enableAllButtons();
    }
}

// CrossrefでDOI検索
void mainDialog::doiSearchCrossref(){
    QString doi = keywordTextEdit->toPlainText();
    //resultTextEdit->setPlainText(QString::fromLocal8Bit("CrossrefでDOI検索を実行開始"));
    statusMessage(QString::fromLocal8Bit("CrossrefでDOI検索を実行開始"));
    disableAllButtons();
    QString url = doiCrossrefBaseUrl + doi;
    doiCrossrefSearchManager->get(QNetworkRequest(QUrl(url)));
}

// doiSearchCrossref()の終了時に実行
void mainDialog::doiSearchCrossrefFinished(QNetworkReply* reply){
    if(reply->error() == QNetworkReply::NoError){
        //qDebug() << tr("success");
        QString searchResultStr = QString::fromUtf8(reply->readAll().data());
        //qDebug() << searchResultStr;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(searchResultStr.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();
        QJsonObject jsonObjSub = jsonObj["message"].toObject();
        QString newFileName = extractFromCrossrefJson(jsonObjSub);

        if (newFileName == tr("  ( 0) ") || newFileName == tr("")){
            //resultTextEdit->setPlainText(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
            statusMessage(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
            enableAllButtons();
        }
        else{
            clipboard->setText(newFileName);
            resultTextEdit->setPlainText(newFileName);
            enableAllButtons();
        }
    }
    else{
        //resultTextEdit->setPlainText(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        statusMessage(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        enableAllButtons();
    }
}

// Crossref検索結果がまとめられたJsonオブジェクトから論文情報抽出
QString mainDialog::extractFromCrossrefJson(QJsonObject j){
    QString journalName;

    int authorNum = j["author"].toArray().size();
    QString firstAuthorFamilyName = j["author"].toArray()[0].toObject()["family"].toString();
    QString lastAuthorFamilyName = j["author"].toArray()[authorNum-1].toObject()["family"].toString();

    QString authors;
    if (authorNum == 1){
        authors = firstAuthorFamilyName;
    }
    else{
        authors = firstAuthorFamilyName + tr(" ") + lastAuthorFamilyName;
    }
    int pubYear = j["created"].toObject()["date-parts"].toArray()[0].toArray()[0].toInt();

    QString paperTitle = j["title"].toArray()[0].toString();

    if (j["type"].toString() == tr("journal-article")){
        journalName = j["short-container-title"].toArray()[0].toString();
        journalName.replace(QString("."), QString(""));
        if (journalName == tr("")) {
            journalName = j["container-title"].toArray()[0].toString();
            statusMessage(QString::fromLocal8Bit("警告: データベースに雑誌の省略名が登録されていません"));
        }
    }
    else if (j["type"].toString() == tr("posted-content")){
        journalName = j["institution"].toObject()["name"].toString();
    }
    else if (j["type"].toString() == tr("proceedings-article")){
        journalName = j["container-title"].toArray()[0].toString();
    }
    QString newFileName = authors + tr(" (") + journalName + tr(" ") + QString::number(pubYear) + tr(") ") + paperTitle;
    newFileName.remove(QRegExp("[/:*\?\"\'<>|]"));
    return newFileName;
}

// Crossref検索結果がまとめられたJsonオブジェクトから論文タイトル抽出
QString mainDialog::extractPaperTitle(QJsonObject j){
    return j["title"].toString();
}
//----End----

//----その他の関数----
// クリップボードの内容を検索結果に更新
void mainDialog::updateClipboard(){
    QString text = resultTextEdit->toPlainText();
    clipboard->setText(text);
}

// 入力欄を使用する検索ボタンの有効化
void mainDialog::enableButtons(){
    QString text = keywordTextEdit->toPlainText();
    text.remove(" ");
    text.remove("\t");
    text.remove("\n");
    if (!(text.isEmpty())){
        keywordSearchButton->setEnabled(true);
        keywordExactSearchButton->setEnabled(true);
        keywordCrossrefSearchButton->setEnabled(true);
        keywordArxivSearchButton->setEnabled(true);
    }
    else{
        keywordSearchButton->setEnabled(false);
        keywordExactSearchButton->setEnabled(false);
        keywordCrossrefSearchButton->setEnabled(false);
        keywordArxivSearchButton->setEnabled(false);
    }
}

// 「クリップボードから検索」ボタンの有効化
void mainDialog::enableClipButton(){
    QString text = clipboard->text();
    text.remove(" ");
    text.remove("\t");
    text.remove("\n");
    if (!(text.isEmpty())){
        clipSearchButton->setEnabled(true);
    }
    else{
        clipSearchButton->setEnabled(false);
    }
}

void mainDialog::enableAllButtons(){
    keywordSearchButton->setEnabled(true);
    keywordExactSearchButton->setEnabled(true);
    keywordCrossrefSearchButton->setEnabled(true);
    keywordArxivSearchButton->setEnabled(true);
    //clipSearchButton->setEnabled(true);
}

void mainDialog::disableAllButtons(){
    keywordSearchButton->setEnabled(false);
    keywordExactSearchButton->setEnabled(false);
    keywordCrossrefSearchButton->setEnabled(false);
    keywordArxivSearchButton->setEnabled(false);
    //clipSearchButton->setEnabled(false);
}

// Levenshtein Distanceの計算
int mainDialog::levenshteinDistance(QString x, QString y){
    int a = x.length();
    int b = y.length();
    int LD[200][200] = {};

    for (int j = 0; j < a; j++) LD[j][0] = j;
    for (int k = 0; k < b; k++) LD[0][k] = k;

    //aをbに近づけたい!
    for (int j = 1; j <= x.size(); j++) {
        for (int k = 1; k <= y.size(); k++) {
            //a[j]を削除するか、a[j+1]にb[k]と同じ文字を挿入するか
            //上記２つの行為の回数で最小な方を採用
            int m = qMin(LD[j - 1][k] + 1, LD[j][k - 1] + 1);
            if (x[j - 1] == y[k - 1]) {
                //最後の文字が同じだから最後の文字がなくても編集距離は同じ
                m = qMin(m, LD[j - 1][k - 1]);
                LD[j][k] = m;
            }
            else {
                //最後の文字を置換する
                m = qMin(m, LD[j - 1][k - 1] + 1);
                LD[j][k] = m;
            }
        }
    }
    return LD[x.size()][y.size()];
}

// QString文字列が全て数値で構成されているか判定
bool mainDialog::checkAllNumber(QString str){
    QRegExp re("\\d*");
    return re.exactMatch(str);
}

// arXivでキーワード検索
void mainDialog::searchArxiv(){
    QString keyword = keywordTextEdit->toPlainText();
    if (keyword.contains(tr("arXiv:"))){
        keyword.remove(tr("arXiv:"));
    }
    resultTextEdit->clear();
    //resultTextEdit->setPlainText(QString::fromLocal8Bit("arXivでキーワード検索を実行開始"));
    statusMessage(QString::fromLocal8Bit("arXivでキーワード検索を実行開始"));
    disableAllButtons();
    keyword.replace(QString(" "), QString("+"));
    QString url = arxivBaseUrl + keyword + tr("%22");
    //qDebug() << url;
    keywordArxivSearchManager->get(QNetworkRequest(QUrl(url)));
}

QString mainDialog::authorNameStripArxiv(QString fullName) {
    int num = fullName.split(" ").size();
    if (num == 2) {
        QString familyName = fullName.split(" ")[1];
        return familyName;
    }
    else {
        QString firstName = fullName.split(" ")[0];
        fullName.remove(firstName + tr(" "));
        int num2 = fullName.split(". ").size();
        QString familyName = fullName.split(". ")[num2-1];
        return familyName;
    }
}

// searchArxiv()の終了時に実行
void mainDialog::arxivSearchFinished(QNetworkReply* reply){
    // Deep Residual Learning for Image Recognition
    if(reply->error() == QNetworkReply::NoError){
        QString searchResultStr = QString::fromUtf8(reply->readAll().data());
        //qDebug() << searchResultStr;
        QDomDocument domDocument;
        domDocument.setContent(searchResultStr);

        QDomNodeList domListTitle = domDocument.elementsByTagName("title");
        QString paperTitle = domListTitle.at(1).toElement().text();
        if (paperTitle == tr("")){
            //resultTextEdit->setPlainText(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
            statusMessage(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
            enableAllButtons();
        }
        else{
        QDomNodeList domListAuthor = domDocument.elementsByTagName("author");
        int authorNum = domListAuthor.size();
        QString firstAuthorFullName = domListAuthor.at(0).toElement().text();
        //qDebug() << firstAuthorFullName;
        //QString firstAuthorFamilyName = firstAuthorFullName.split(" ")[1];
        QString firstAuthorFamilyName = authorNameStripArxiv(firstAuthorFullName);
        //qDebug() << firstAuthorFamilyName;
        QString lastAuthorFullName = domListAuthor.at(authorNum - 1).toElement().text();
        //QString lastAuthorFamilyName = lastAuthorFullName.split(" ")[1];
        QString lastAuthorFamilyName = authorNameStripArxiv(lastAuthorFullName);

        QString authors;
        if (authorNum == 1) {
            authors = firstAuthorFamilyName;
        }
        else {
            authors = firstAuthorFamilyName + tr(" ") + lastAuthorFamilyName;
        }

        QDomNodeList domListYear = domDocument.elementsByTagName("published");
        QString pubYear = domListYear.at(0).toElement().text().left(4);
        //qDebug() << pubYear;

        QString newFileName = authors + tr(" (arXiv ") + pubYear + tr(") ") + paperTitle;
        //qDebug() << newFileName;
        newFileName.remove(QRegExp("[/:*\?\"\'<>|]"));
        clipboard->setText(newFileName);
        resultTextEdit->setPlainText(newFileName);
        enableAllButtons();
        }
    }
    else{
        //resultTextEdit->setPlainText(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        statusMessage(QString::fromLocal8Bit("エラー: 文献が見つかりませんでした"));
        enableAllButtons();
    }
}

bool mainDialog::checkDoi(QString str){
    QRegExp re("^[0-9]*\\..*/*");
    return re.indexIn(str) != -1;
}
//----End----
