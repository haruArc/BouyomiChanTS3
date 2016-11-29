#ifndef BouyomiConfigDialog_H
#define BouyomiConfigDialog_H

#include <QDialog>

namespace Ui {
class BouyomiConfigDialog;
}

class BouyomiConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BouyomiConfigDialog(QWidget *parent = 0);
    ~BouyomiConfigDialog();

private:
    Ui::BouyomiConfigDialog *ui;

private slots:
    void accept();
    void reject();
};

#endif // BouyomiConfigDialog_H
