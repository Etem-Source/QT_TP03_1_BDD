#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), label(new QLabel(this))
{
    ui->setupUi(this);
    setWindowTitle("Authentication");
    resize(400, 400);

    // Configuration de la base de données
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("Utilisateurs_TP");
    db.setUserName("qttp03");
    db.setPassword("Qttp03??");

    if (!db.open())
    {
        QMessageBox::critical(this, "Database Connection Error", db.lastError().text());
        return;
    }

    // Pavé numérique avec génération aléatoire
    QList<int> numbers;
    for (int i = 0; i < 10; ++i)
        numbers.append(i);
    std::random_shuffle(numbers.begin(), numbers.end(), [](int i) { return QRandomGenerator::global()->bounded(i); });

    for (int i = 0; i < 10; ++i)
    {
        QString text = QString::number(numbers[i]);
        buttons[numbers[i]] = new QPushButton(text, this);
        connect(buttons[numbers[i]], &QPushButton::pressed, this, [this, text]() {
            for (int j = 0; j < 10; ++j)
            {
                if (buttons[j]->text() == text)
                {
                    buttons[j]->setStyleSheet("color: red");
                }
            }
        });
        connect(buttons[numbers[i]], &QPushButton::released, this, [this, text]() {
            for (int j = 0; j < 10; ++j)
            {
                if (buttons[j]->text() == text)
                {
                    buttons[j]->setStyleSheet("");
                }
            }
        });
        connect(buttons[numbers[i]], &QPushButton::clicked, this, &MainWindow::slot_buttonClicked);
    }

    // Layout Pavé numérique
    QGridLayout *layout = new QGridLayout();
    layout->setMargin(6);
    layout->setSpacing(6);
    for (int i = 0; i < 9; i++)
    {
        layout->addWidget(buttons[numbers[i + 1]], i / 3, i % 3);
    }
    layout->addWidget(buttons[numbers[0]], 3, 1);

    // Champs ID et Mot de passe
    idField = new QLineEdit(this);
    idField->setPlaceholderText("Identifiant");
    idField->setMaxLength(9);
    connect(idField, &QLineEdit::textChanged, this, [this]() {
        passwordField->setEnabled(false);
        messageLabel->clear();
        loginButton->setStyleSheet("");
        // Check if the entered ID exists in the database
        QSqlQuery query;
        query.prepare("SELECT COUNT(*) FROM identifiants WHERE id = :id");
        query.bindValue(":id", idField->text());
        if (query.exec() && query.next() && query.value(0).toInt() > 0)
        {
            passwordField->setEnabled(true);
        }
    });

    passwordField = new QLineEdit(this);
    passwordField->setPlaceholderText("Mot de passe");
    passwordField->setEchoMode(QLineEdit::Password);
    passwordField->setEnabled(false);

    // Case à cocher pour montrer/masquer le mot de passe
    showPasswordCheckBox = new QCheckBox("Afficher Mot de passe", this);
    connect(showPasswordCheckBox, &QCheckBox::toggled, this, &MainWindow::slot_togglePassword);

    // Label pour afficher le message
    messageLabel = new QLabel(this);
    messageLabel->setAlignment(Qt::AlignCenter);

    // Bouton Connexion
    loginButton = new QPushButton("Connexion", this);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::slot_login);

    // Ajouter les widgets au layout
    layout->addWidget(idField, 4, 0, 1, 3);
    layout->addWidget(passwordField, 5, 0, 1, 3);
    layout->addWidget(showPasswordCheckBox, 6, 0, 1, 3);
    layout->addWidget(loginButton, 7, 0, 1, 3);
    layout->addWidget(messageLabel, 8, 0, 1, 3);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);
}

void MainWindow::slot_buttonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button)
    {
        idField->insert(button->text());
    }
}

void MainWindow::slot_togglePassword()
{
    if (showPasswordCheckBox->isChecked())
    {
        passwordField->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        passwordField->setEchoMode(QLineEdit::Password);
    }
}

void MainWindow::slot_login()
{
    QSqlQuery query;
    query.prepare("SELECT mdp, nom, prenom FROM identifiants WHERE id = :id");
    query.bindValue(":id", idField->text());

    if (query.exec() && query.next())
    {
        QString dbPassword = query.value(0).toString();
        QString nom = query.value(1).toString();
        QString prenom = query.value(2).toString();
        if (passwordField->text() == dbPassword)
        {
            messageLabel->setText(QString("Connexion réussie! Bonjour, %1 %2").arg(prenom, nom));
            loginButton->setStyleSheet("color: green");
            messageLabel->setStyleSheet("color: green");
        }
        else
        {
            essaies--;
            messageLabel->setText(QString("Mot de passe incorrect! Tentatives restantes : %1").arg(essaies));
            messageLabel->setStyleSheet("color: red");
            loginButton->setStyleSheet("color: red");
            if (essaies == 0)
            {
                close(); // Ferme l'application après 3 tentatives infructueuses
            }
        }
    }
    else
    {
        messageLabel->setText("Identifiant incorrect!");
        loginButton->setStyleSheet("color: red");
        messageLabel->setStyleSheet("color: red");
        passwordField->setEnabled(false); // Disable password field if ID is incorrect
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
