#include "widget.h"
#include "ui_widget.h"
#include "mainDialog.h"
#include <QtGui>
#include <QtWidgets>
#include <QMenuBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QPlainTextEdit>
#include <QTextCodec>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent){
    maindialog = new mainDialog;
    setCentralWidget(maindialog); // メインウィジェットに設定
    setWindowTitle(tr("PubmedSearch2FileName"));
    setFixedHeight(250);
    setFixedWidth(610);

    // アクションの作成
    createActions();
    // メニューバーの作成
    createMenus();
    createStatusBar();
    setAcceptDrops(true);

    // アプリ左上に表示されるアイコンの設定
    setWindowIcon(QIcon(":/app.ico"));

    // アプリの初期位置を前回開いたときの位置に移動
    QSettings settings("Yakusaku", "PubmedSearch2FileName");
    restoreGeometry(settings.value("geometry").toByteArray());

    connect(maindialog, SIGNAL(statusMessage(QString)), this, SLOT(printInfoToStatusBar(QString)));
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event){
    const QMimeData* mime = event->mimeData();
    if (mime->hasFormat("text/uri-list")){
        if (mime->urls().size() != 1){
            event->ignore();
        }
        else if (mime->urls().first().isLocalFile()){
            if (mime->text().contains(tr(".pdf")) || mime->text().contains(tr(".PDF"))){
                event->acceptProposedAction();
            }
            else {
                event->ignore();
            }
        }else if (mime->hasText()){
            event->acceptProposedAction();
        }
        else{
            event->ignore();
        }
    }
    else if (mime->hasText()){
        event->acceptProposedAction();
    }
    else{
        event->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *event){
    const QMimeData* mime = event->mimeData();
    if (mime->hasFormat("text/uri-list")){
        if (mime->urls().first().isLocalFile()){
            QString fileName = QFileInfo(mime->urls().first().toLocalFile()).baseName();
            maindialog->keywordTextEdit->setPlainText(fileName);
            maindialog->resultTextEdit->clear();
            maindialog->searchPubmed();
        }
        else{
            QString text = mime->text();
            if (text.contains(tr("https://doi.org/"))){
                text.remove(tr("https://doi.org/"));
            }
            maindialog->keywordTextEdit->setPlainText(text);
            maindialog->resultTextEdit->clear();
            maindialog->searchPubmed();
        }
    }
    else if (mime->hasText()){
        QString text = mime->text();
        if (text.contains(tr("https://doi.org/"))){
            text.remove(tr("https://doi.org/"));
        }
        maindialog->keywordTextEdit->setPlainText(text);
        maindialog->resultTextEdit->clear();
        maindialog->searchPubmed();
    }
}

void MainWindow::closeEvent(QCloseEvent *event){
    QSettings settings("Yakusaku", "PubmedSearch2FileName");
    settings.setValue("geometry", saveGeometry());
    QWidget::closeEvent(event);
}

void MainWindow::createActions(){
    aboutQtAction = new QAction(tr("About &Qt"),this);
    connect(aboutQtAction, SIGNAL(triggered()),qApp, SLOT(aboutQt()));

    openAction = new QAction(tr("&Open"), this);
    openAction->setShortcut(tr("Ctrl+O"));
    connect(openAction, SIGNAL(triggered()),this, SLOT(open()));

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setShortcut(tr("Ctrl+H"));
    connect(aboutAction, SIGNAL(triggered()),this, SLOT(about()));
}

void MainWindow::createMenus(){
    fileMenu = QMainWindow::menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    helpMenu = QMainWindow::menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::createStatusBar(){
    //statusLabel = new QLabel("");
    //statusLabel->setIndent(7);
    //statusBar()->addWidget(statusLabel);
    statusBar = new QStatusBar;
    statusBar->setObjectName(QString::fromUtf8("statusBar"));
    setStatusBar(statusBar);
}

void MainWindow::printInfoToStatusBar(QString msg){
    statusBar->showMessage(msg,2000);
}

// PDFファイルを開いて入力欄にファイル名を表示
void MainWindow::open(){
    QString fileName = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("リネームしたい論文PDFを指定"), ".",
                                           tr("PDF files (*.pdf *.PDF)"));
    if (!fileName.isEmpty()){
        QFileInfo info(fileName);
        QString fileBaseName = info.completeBaseName(); // 拡張子とパスを除いたファイル名
        maindialog->plainTextEdit->setPlainText(fileBaseName);
        maindialog->plainTextEdit_2->clear();
        maindialog->searchPubmedKeyword();
    }
}

void MainWindow::about(){
    QDialog *childDialog = new QDialog(this);

    childDialog->setWindowTitle(tr("PubmedSearch2FileName"));
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *help = new QLabel(this);
    help->setText(QString::fromLocal8Bit(
                "<h3>概要</h3>"
                "<p style='line-height: 150%'>"
                "論文名 or PMID or DOIを入力して検索すると、規定のフォーマットでファイル名を生成するアプリです。<br>"
                "アメリカ国立生物工学情報センター(NCBI)が提供するAPIサービスの「E-utilities」、<br>"
                "及びDOIの公式登録機関であるCrossRefが提供するAPIサービスの「Crossref REST API」、<br>"
                "及びコーネル大学のComputing and Information Scienceが提供しているプレプリントサーバーであるarXivのAPIサービスの「arXiv API」を利用しています。</p>"
                "<h3>論文検索方法について</h3>"
                "<p style='line-height: 150%'>"
                "以下の8通りの方法で論文検索を行うことができます。大きく分けると2通りに分かれます。<br>"
                "片方はアプリのボタンを使って検索する方法、もう片方はPDFファイルを扱う方法です。<br>"
                "前者の場合、上側の入力欄には半角文字のみ入力してください。後者の場合、あらかじめファイル名を論文タイトルに変更しておいてください。<br><br>"
                "1) 入力欄に PMID or 論文名（キーワードでも可） or DOI を入力し、「入力欄から検索」ボタンを押す。<br>"
                "2) 検索したいキーワード等をクリップボードに入れた（＝コピーした）上で、「クリップボードから検索」ボタンを押す。<br>"
                "※アプリ初回起動時にクリップボードにデータが存在する場合、自動で検索を行います。<br>"
                "3) アプリ上側のファイルタブ > 開くによって論文PDFを選択する。<br>"
                "4) アプリ起動前に、アプリアイコン上にPDFをドラッグ＆ドロップする。<br>"
                "※クリップボードの読み取りより優先して実行されます。<br>"
                "5) アプリ起動後に、ウィンドウ上にPDFをドラッグ＆ドロップする。<br>"
                "6) 入力欄にキーワード等を入力し、「入力欄から高精度検索」ボタンを押す。<br>"
                "※(1)より正確な検索を行える代わりに検索時間がかかります。(1-5)の方法で検索に失敗したときにお使いください。<br>"
                "7) 入力欄にキーワード等を入力し、「CrossRef検索」ボタンを押す。<br>"
                "※Pubmedには存在しないがDOIは有する論文 (Conference paperやbioRxiv等) の検索にお使いください。<br>"
                "※bioRxiv版とjournal版が存在する場合、Pubmed検索を行う方が確実です。<br>"
                "※(1-6)の方法で検索に失敗したとき、自動で実行されます。<br>"
                "8) 入力欄にキーワード等を入力し、「arXiv検索」ボタンを押す。</p>"
                "<h3>検索結果について</h3>"
                "<p style='line-height: 150%'>"
                "検索後に、「第一著者のファミリーネーム 最終著者のファミリーネーム (論文誌名 出版年) 論文名」という順で論文ファイル名を生成します。<br>"
                "例) Ikegaya Matsumoto (Science 2019) Spikes in the sleeping brain<br>"
                "検索結果は下側の入力欄に表示されます。検索がヒットしなかった場合、代わりにエラーメッセージが表示されます。<br>"
                "検索結果は自動でクリップボードにコピーされます。検索結果の表示後、下側の入力欄の編集内容は自動でクリップボードに反映されます。<br>"
                "入力欄を用いない検索方法の場合、検索に用いたキーワード等が入力欄に表示されます。<br>"
                "※Conference paperの場合は仕様上、会議の正式名称が論文誌名の代わりに表示されます。<br>"
                "※データベースに雑誌の省略名が登録されていない場合、雑誌の正式名称が表示されます。<br>"
                "※「van der ~」のようにファミリーネーム中に前置詞が含まれる場合、前置詞も含めて表示されます。<br>"
                "※基本的に一番最初にヒットした論文をもとにファイル名を生成するため、必ずしも正しい論文が選択されるわけではありません。</p>"
                "<h3>連絡先について</h3>"
                "<p style='line-height: 150%'>"
                "何か不具合や要望がありましたら、 korita.utyaku502171＠gmail.com までご連絡ください。<br>"
                "また、http／／github.com／korita1995／PubmedSearch2FileNameV2 でソースファイルを公開しています。<br>"
                "製作者: 折田 健 (Orita Ken)</p>"
                ));
    layout->addWidget(help);
    childDialog->setLayout(layout);
    childDialog->show();
}
