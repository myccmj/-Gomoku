#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <vector>
#include <QMouseEvent>
#include <iostream>
#include <math.h>
#define N 15
#define rowWid 50


using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
//棋子
struct qz{
    int i,j,P=0;
    float score[4]={0};
    int another_i[4],another_j[4],block[4]={0};
    qz()
    {}
    qz(int _i,int _j,int _p)
    {
        i=_i;
        j=_j;
        P=_p;
    }
    qz(const qz & q){
            memcpy(this,&q,sizeof(qz));
            //cout<<"Copy Constructor called"<<endl ;
        }
    void set(int _i,int _j,int _p)
    {
        i=_i;
        j=_j;
        P=_p;
    }
    void set1(const qz & q)
    {
        int tempP=P;
        memcpy(this,&q,sizeof(qz));
        P=tempP;
    }
    void set1(int drc,float _score,int _block,int _another_i=-1,int _another_j=-1)
    {
        score[drc]=_score;
        if(_another_i!=-1)
        {
        another_i[drc]=_another_i;
        another_j[drc]=_another_j;
        }
        block[drc]=_block;
    }
    void reset()
    {
        int tempi=i,tempj=j;
        memset(this,0,sizeof(qz));
        i=tempi;j=tempj;
    }
    void print(bool _score=0,bool _block=0,bool _anotherIJ=0)const
    {
        cout<<"i:"<<i<<" j:"<<j<<" P:"<<P<<endl;
        if(_score)
        {
            cout<<"score:"<<score[0]<<" "<<score[1]<<" "<<score[2]<<" "<<score[3]<<endl;
        }
        if(_block)
        {
            cout<<"block:"<<block[0]<<" "<<block[1]<<" "<<block[2]<<" "<<block[3]<<endl;
        }
        if(_anotherIJ)
        {
            cout<<"another_ij:"<<another_i[0]<<","<<another_j[0]<<" "<<another_i[1]<<","<<another_j[1]<<" "
               <<another_i[2]<<","<<another_j[2]<<" "<<another_i[3]<<","<<another_j[3]<<endl;
        }
    }
    int getlen(int drc)
    {
        return max(abs(another_i[drc]-i),abs(another_j[drc]-j))+1;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *m);
    int findqz(int i,int j)
    {
        for(int k=0;k<QZs.size();k++)
        {
            if(i==QZs[k].i&&j==QZs[k].j)
                return k;
        }
        return -1;
    }
    void AI();
    void AI1();
    void AI2();
    void getCandidate(vector<qz>&,int);
    int board[N][N]={0};
    vector<qz> QZs;
    vector<qz> predict_QZs;
    int static crtP,P_AI;
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
    //for test
    vector<qz> editedQZs;
};
#endif // MAINWINDOW_H
