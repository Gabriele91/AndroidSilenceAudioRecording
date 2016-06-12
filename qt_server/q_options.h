#ifndef Q_OPTIONS_H
#define Q_OPTIONS_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class q_options;
}

class q_options : public QDialog
{
    Q_OBJECT

public:

    explicit q_options(QWidget *parent = 0);

    ~q_options();

    QString get_path() const;

    unsigned short get_port() const;

    unsigned short get_max_clients() const;

    int exec();

    void set_port(int port);

    void set_path(const QString& path);

public slots:

    void set_path_from_ui();
    void accepted();
    void rejected();

private:

    //close state
    int m_close_state;
    //setting class
    QSettings m_settings;
    //save last path opened
    QString m_last_path;
    //ui ref
    Ui::q_options *m_ui;

};

#endif // Q_OPTIONS_H
