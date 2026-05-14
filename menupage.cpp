#include "menupage.h"
#include "ui_menupage.h"
#include <QShortcut>
#include <QScreen>
// Add this include

MenuPage::MenuPage(Database *db,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MenuPage),m_db(db), m_userId(-1)
{
    ui->setupUi(this);
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    resize(screenGeometry.width(), screenGeometry.height());

    // Force window to be maximized
    setWindowState(windowState() | Qt::WindowMaximized);

    // Enable Escape key to close
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcut, &QShortcut::activated, this, &QDialog::close);
    setWindowFlags(windowFlags() | Qt::WindowCloseButtonHint);

#include <QShortcut> // Add this include

    ui->lblRules->setText(
        "Rules:\n"
        " - Flip cards and pair them up.\n"
        " - Score increases when you match pairs.\n"
        " - There are 3 levels (Easy/Medium/Hard) that set total time.\n"
        " - Special pair gives +5 seconds. Bomb card subtracts -2 seconds.\n"
        " - Try to match needed pairs before time runs out.");
   // connect(ui->btnStart, &QPushButton::clicked, this, &MenuPage::on_btnStart_clicked);
    //connect(ui->btnLeaderboard, &QPushButton::clicked, this, &MenuPage::on_btnLeaderboard_clicked);
    //connect(ui->btnLogout, &QPushButton::clicked, this, &MenuPage::on_btnLogout_clicked);

}

MenuPage::~MenuPage()
{
    delete ui;
}
void MenuPage::setUser(int userId, const QString &username)
{
    m_userId = userId;
    m_username = username;
    ui->lblWelcome->setText(QString("Welcome, %1").arg(username));
}

void MenuPage::on_btnStart_clicked()
{
    qDebug() << "Start button clicked - emitting startGame signal";
    int levelIndex = ui->cbLevel->currentIndex() - 1;
    if (levelIndex < 0) levelIndex = 0;
    emit startGame(m_userId, levelIndex);
}


void MenuPage::on_btnLeaderboard_clicked()
{
    qDebug() << "Leaderboard button clicked - emitting showLeaderboard signal";
    emit showLeaderboard();
}


void MenuPage::on_btnLogout_clicked()
{
 emit logoutRequested();
}

