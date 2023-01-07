#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "randomc.h"
#include <unordered_map>
#define D 4
#define SCORE_5 1000000

using namespace std;
int MainWindow::crtP=0,MainWindow::P_AI=1;
float P1_SCORE,P2_SCORE,PREHENSIVE_SCORE;
int Imin=N,Imax=-1,Jmin=N,Jmax=-1;
bool alpha_beta_test=1,zorist_test=0;
int predict_paint_N=0,secondery_paint_N=0;
bool AIfirst=0;
long long zobrist0[N][N],zobrist1[N][N];
vector<qz> secondery_predict;
//for test
float last_s0=0,last_s1=0;

inline uint64_t get_rand64(CRandomMersenne &RanGen)
{
    return
        (((uint64_t) RanGen.IRandom(0,INT_MAX) <<  0) & 0x00000000FFFFFFFFull) |
        (((uint64_t) RanGen.IRandom(0,INT_MAX) << 32) & 0xFFFFFFFF00000000ull);
}
struct ZobristNode{
    float score=0;
    bool fixed=0;
    ZobristNode()
    {}
    ZobristNode(float _score,bool _fixed)
    {
        score=_score;
        fixed=_fixed;
    }
};

struct Zobrist{

    unordered_map<long long,ZobristNode> map[D-2];
    unordered_map<long long,string> test_map;
    long long get_hashkey(int i,int j,int P)
    {
        //P=0,1
        if(P)
            return zobrist1[i][j];
        else
            return zobrist0[i][j];
    }
    ZobristNode get_value(long long hashkey,int d)
    {
        d-=3;
        return map[d][hashkey];
    }
    bool set_value(long long hashkey,int d,float score,bool fixed,string path="")
    {
        d-=3;
//        if(map[d].count(hashkey))
//            return false;
        map[d][hashkey]=ZobristNode(score,fixed);
        if(zorist_test)
        if(path!="")
            test_map[hashkey]=path;
        return true;
    }
    void clear()
    {
        for(int i=0;i<D-2;i++)
        {
            map[i].clear();
        }
    }

};
Zobrist zobrist;
struct Node{
    float alpha=INT_MIN,beta=INT_MAX,score=0.12345;
    bool fixed=0,visited=0;
    Node* parent=nullptr,*best_child;
    int i,j,d=0,P;
    bool ifDelete=1;
    Node()
    {
    }
    Node(int _i,int _j,int _d,int _P,Node* par=nullptr)
    {
        i=_i;
        j=_j;
        d=_d;
        P=_P;
        parent=par;
    }
    void setScore(float _score=NULL)
    {
        fixed=1;
        if(!_score)
        {
        if(d%2)
            score=beta;
        else
            score=alpha;
        }
        else
            score=_score;
    }
//    bool setZobrist(Zobrist &_zobrist)
//    {
//        return _zobrist.set_value(hashkey,d,score);
//    }

    bool setParAlphaBeta()
    {
        //当前层MIN，父亲层MAX
        if(d%2)
        {
            if(fixed)
            {
                if(score>parent->alpha)
                {
                    parent->alpha=score;
                    if(d==1)
                    {
                        parent->i=i;parent->j=j;
                    }
                    if(alpha_beta_test)
                    {
                        parent->best_child=this;
                        ifDelete=false;
                    }

                }
            }
        }
        else
        {
            if(fixed)
            {
                //parent->beta=min(parent->beta,score);
                if(score<parent->beta)
                {
                    parent->beta=score;
                    if(d==1)
                    {
                        parent->i=i;parent->j=j;
                    }
                    if(alpha_beta_test)
                    {
                        parent->best_child=this;
                        ifDelete=false;
                    }
                }
            }
        }
        if(parent->alpha>=parent->beta)
            return true;
        return false;
    }
    void setAlphaBetafromPar()
    {
        //当前层MIN，父亲层MAX
//        if(d%2)
//        {
            alpha=parent->alpha;
//        }
//        else
            beta=parent->beta;
    }
};

void print(vector<qz> QZs)
{
    cout<<"QZs:";
    for(int i=0;i<QZs.size();i++)
    {
        QZs[i].print(1,0,0);
    }
}
inline void changeIJ(int &i,int &j,int drc,int reverse=0)
{
    if(!reverse)
    {
        if(drc==0)
            j--;
        else if(drc==1){
            i--;
            j--;
        }
        else if(drc==2)
        {
            i--;
        }
        else
        {
            i--;
            j++;
        }
    }
    else
    {
        if(drc==0)
            j++;
        else if(drc==1){
            i++;
            j++;
        }
        else if(drc==2)
        {
            i++;
        }
        else
        {
            i++;
            j--;
        }
    }
}
inline bool IJinBoard(int i,int j)
{
    if(i>=0&&j>=0&&i<N&&j<N)
        return true;
    return false;
}
struct IntelligentBoard{
    qz board[N][N];
    long long hashkey=0;
    bool candidateQZ[N][N]={0};
    void printCandidate()
    {
        for(int i=0;i<N;i++)
        {
            for(int j=0;j<N;j++)
            {
                cout<<candidateQZ[i][j]<<" ";
            }
            cout<<endl;
        }
        cout<<"//////////////////////"<<endl;
    }
    vector<int> setCandidateQZ(int I,int J,int d=1)
    {
        vector<int> setQZs;
        for(int i=max(0,I-d);i<min(N,I+d+1);i++)
            for(int j=max(0,J-d);j<min(N,J+d+1);j++)
            {
                if(!candidateQZ[i][j])
                {
                    setQZs.push_back(i);setQZs.push_back(j);
                    candidateQZ[i][j]=1;
                }
            }
        return setQZs;
    }
    void setCandidateQZ_reverse(vector<int> &setCandidateQZs)
    {
        for(int i=0;i<setCandidateQZs.size();i+=2)
        {
            int I=setCandidateQZs[i],J=setCandidateQZs[i+1];
            candidateQZ[I][J]=0;
        }
    }
    long long get_hashkey(vector<qz> &QZs)
    {
        hashkey=0;
        for(int i=0;i<QZs.size();i++)
        {
            int I=QZs[i].i,J=QZs[i].j,P=QZs[i].P-1;
            hashkey^=zobrist.get_hashkey(I,J,P);
            return hashkey;
        }
    }
    void setQZfromQZs(vector<qz> &editedQZs)
    {
//        cout<<"setQZfromQZs"<<endl;
//        bool flag[N][N]={0};
        for(int i=0;i<editedQZs.size();i++)
        {
            int I=editedQZs[i].i,J=editedQZs[i].j;
//            if(flag[I][J])
//                cout<<"REPEATED!!!!!!!!!!!!!!"<<endl;
//            editedQZs[i].print();
            board[I][J].set1(editedQZs[i]);
//            flag[I][J]=1;
        }
    }
    void clear()
    {
        for(int i=0;i<N;i++)
            for(int j=0;j<N;j++)
                board[i][j].reset();
    }
    void clearCandidate()
    {
        memset(candidateQZ,0,sizeof(candidateQZ));
    }
    vector<int> calMaxLen(int I,int J,int P,bool flagBoard[N][N][4],int drc)
    {

        //P=1或2
        int i=I,j=J;
        flagBoard[i][j][drc]=1;
        changeIJ(i,j,drc);
        int len=1,block=0;
        while(i>=0&&j>=0&&i<N&&j<N)
        {
            if(flagBoard[i][j][drc]&&board[i][j].P==P)
            {
                return {-1,-1};
            }
            if(!board[i][j].P)
            {
//                if(!empty)
//                {
//                    int i1=i,j1=j;
//                    changeIJ(i1,j1,drc);
//                    if(IJinBoard(i1,j1)&&board[i1][j1].P==P)
//                    {
//                        empty++;
//                        i=i1;
//                        j=j1;
//                    }
//                    else
//                        break;
//                }
//                else
                    break;
            }
            if(board[i][j].P!=P)
            {
                block++;
                break;
            }

            len++;
            if(board[i][j].P)
                flagBoard[i][j][drc]=1;
            changeIJ(i,j,drc);
        }
        if(!(i>=0&&j>=0&&i<N&&j<N))
        {
            block++;
        }
        int I0=i,J0=j;
        changeIJ(I0,J0,drc,1);
        i=I;j=J;
        changeIJ(i,j,drc,1);
        while(i>=0&&j>=0&&i<N&&j<N)
        {
            if(flagBoard[i][j][drc]&&board[i][j].P==P)
            {
                return {-1,-1};
            }
            if(!board[i][j].P)
            {
//                if(!empty)
//                {
//                    int i1=i,j1=j;
//                    changeIJ(i1,j1,drc,1);
//                    if(IJinBoard(i1,j1)&&board[i1][j1].P==P)
//                    {
//                        empty++;
//                        i=i1;
//                        j=j1;
//                    }
//                    else
//                        break;
//                }
//                else
                    break;
            }
            if(board[i][j].P!=P)
            {
                block++;
                break;
            }
            len++;
            if(board[i][j].P)
                flagBoard[i][j][drc]=1;
            changeIJ(i,j,drc,1);
        }
        if(!(i>=0&&j>=0&&i<N&&j<N))
        {
            block++;
        }
        int I1=i,J1=j;
        changeIJ(I1,J1,drc);
        return {len,block,0,I0,J0,I1,J1};
    }
    vector<float> Score(const vector<qz> &QZs,bool print=0,int set=1,bool sethash=0,bool setCandidate=0)
    {
        if(sethash)
            hashkey=0;
        //计算场上双方的分数
        bool flagBoard[N][N][4]={0};
        float score[2]={0};
        for(int i=0;i<QZs.size();i++)
        {
            const qz &temp=QZs[i];
            int I=temp.i,J=temp.j,P=temp.P;
            if(sethash)
            {
                hashkey^=zobrist.get_hashkey(I,J,P-1);
            }
            if(setCandidate)
            {
                setCandidateQZ(I,J,2);
            }
            for(int drc=0;drc<=3;drc++)
            {
                if(flagBoard[I][J][drc])
                    continue;
                vector<int> tmp=calMaxLen(I,J,P,flagBoard,drc);
                int len=tmp[0],block=tmp[1],empty=tmp[2],I0=tmp[3],J0=tmp[4],I1=tmp[5],J1=tmp[6];

                if(len==-1)
                    continue;
                float s=0;
                if(len>=5)
                {
                    s=SCORE_5;
//                    if(empty)
//                    {
//                        s=1000;
//                        if(block)
//                            s=100;
//                    }
                }
                else
                {
    //                if(empty&&len<3)
    //                    continue;
                    s=pow(10,len-1-block);
                    if(block>=2)
                        s=0;
//                    if(empty)
//                        s*=0.5;
                }

//                if(print&&len>1)
//                    cout<<"drc:"<<drc<<" len:"<<len<<" block:"<<block<<" empty:"<<empty<<" P:"<<P<<" s:"<<s<<" I:"<<I<<" J:"<<J<<endl;
                score[P-1]+=s;
                if(set)
                {
                board[I0][J0].set1(drc,s,block,I1,J1);
                board[I1][J1].set1(drc,s,block,I0,J0);
                }
            }
            flagBoard[I][J][0]=1;flagBoard[I][J][1]=1;flagBoard[I][J][2]=1;flagBoard[I][J][3]=1;
        }
        if(print)
            cout<<score[0]<<" "<<score[1]<<"/////////////////"<<endl;
        return {score[0],score[1]};
    }
    vector<float> Score_local(const vector<qz> &QZs,int start,vector<qz> &editedQZs,bool set=1,bool print=0)
    {
        editedQZs.clear();
        //计算场上双方的分数
        bool flagBoard[N][N][4]={0};
        float score[2]={0};
        for(int i=start;i<QZs.size();i++)
        {
            const qz &temp=QZs[i];
            //temp.print();
            int I=temp.i,J=temp.j,P=temp.P;
            int neg_P=!(P-1)+1;
            for(int drc=0;drc<=3;drc++)
            {
                I=temp.i;J=temp.j;
                if(flagBoard[I][J][drc])
                    continue;
                vector<int> tmp=calMaxLen(I,J,P,flagBoard,drc);
                int len=tmp[0],block=tmp[1],empty=tmp[2],I0=tmp[3],J0=tmp[4],I1=tmp[5],J1=tmp[6];

                if(len==-1)
                    continue;
                float s=0;
                if(len>=5)
                {
                    s=SCORE_5;
//                    if(empty)
//                    {
//                        s=1000;
//                        if(block)
//                            s=100;
//                    }
                }
                else
                {
    //                if(empty&&len<3)
    //                    continue;
                    s=pow(10,len-1-block);
                    if(block>=2)
                        s=0;
//                    if(empty)
//                        s*=0.5;
                }

//                if(print&&len>1)
//                    cout<<"Local drc:"<<drc<<" len:"<<len<<" block:"<<block<<" empty:"<<empty<<" P:"<<P<<" s:"<<s<<" I:"<<I<<" J:"<<J<<endl;
//                if(len>1)
//                cout<<I0<<" "<<J0<<" "<<I1<<" "<<J1<<" "<<I<<" "<<J<<endl;
                if((I!=I0||J!=J0)&&(I!=I1||J!=J1))
                {
                    score[P-1]-=board[I0][J0].score[drc]+board[I1][J1].score[drc];
                    score[P-1]+=s;
                    if(set)
                    {
                        editedQZs.push_back(board[I0][J0]);editedQZs.push_back(board[I1][J1]);
                    board[I0][J0].set1(drc,s,block,I1,J1);board[I1][J1].set1(drc,s,block,I0,J0);

                    }
                }
                else if(I==I0&&J==J0)
                {
                    score[P-1]+=s-board[I1][J1].score[drc];
                    if(set)
                    {
                        if(len>1)
                        {
                        editedQZs.push_back(board[I1][J1]);
                        board[I1][J1].set1(drc,s,block,I0,J0);
                        }
                    board[I0][J0].set1(drc,s,block,I1,J1);

                    }
                }
                else if(I==I1&&J==J1)
                {
                    score[P-1]+=s-board[I0][J0].score[drc];
                    if(set)
                    {
                        if(len>1)
                        {
                        editedQZs.push_back(board[I0][J0]);
                    board[I0][J0].set1(drc,s,block,I1,J1);
                        }
                    board[I1][J1].set1(drc,s,block,I0,J0);

                    }
                }
                changeIJ(I,J,drc);
                if(IJinBoard(I,J))
                {
                    qz &neg_q=board[I][J];
                if(neg_q.P==neg_P)
                {
                    if(neg_q.block[drc])
                    {
                        score[neg_P-1]-=neg_q.score[drc];
                        int another_i=neg_q.another_i[drc],another_j=neg_q.another_j[drc];
                        if(set)
                        {
                            editedQZs.push_back(neg_q);
                            if(neg_q.getlen(drc)>1)
                            {
                                editedQZs.push_back(board[another_i][another_j]);
                                board[another_i][another_j].set1(drc,0,2);
                            }
                        neg_q.set1(drc,0,2);
                        }
                    }
                    else
                    {
                        float tempScore=neg_q.score[drc];

                        int another_i=neg_q.another_i[drc],another_j=neg_q.another_j[drc];

                        score[neg_P-1]-=tempScore*0.9;
                        if(set)
                        {
                            editedQZs.push_back(neg_q);
                            if(neg_q.getlen(drc)>1)
                            {
                            editedQZs.push_back(board[another_i][another_j]);
                            board[another_i][another_j].set1(drc,tempScore/10,1);
                            }
                        neg_q.set1(drc,tempScore/10,1);
                        }
                    }
                }
                }
                I=temp.i;J=temp.j;
                changeIJ(I,J,drc,1);
                if(IJinBoard(I,J))
                {
                    qz &neg_q=board[I][J];
                if(neg_q.P==neg_P)
                {
                    if(neg_q.block[drc])
                    {
                        score[neg_P-1]-=neg_q.score[drc];
                        int another_i=neg_q.another_i[drc],another_j=neg_q.another_j[drc];
                        if(set)
                        {
                            editedQZs.push_back(neg_q);
                            if(neg_q.getlen(drc)>1)
                            {
                                editedQZs.push_back(board[another_i][another_j]);
                                board[another_i][another_j].set1(drc,0,2);
                            }
                        neg_q.set1(drc,0,2);
                        \
                        }
                    }
                    else
                    {
                        float tempScore=neg_q.score[drc];
                        int another_i=neg_q.another_i[drc],another_j=neg_q.another_j[drc];
                        score[neg_P-1]-=tempScore*0.9;
                        if(set)
                        {
                            editedQZs.push_back(neg_q);
                            if(neg_q.getlen(drc)>1)
                            {
                                editedQZs.push_back(board[another_i][another_j]);
                                board[another_i][another_j].set1(drc,tempScore/10,1);
                            }
                        neg_q.set1(drc,tempScore/10,1);

                        }
                    }
                }
                }
            }
            flagBoard[I][J][0]=1;flagBoard[I][J][1]=1;flagBoard[I][J][2]=1;flagBoard[I][J][3]=1;
        }
        if(print)
            cout<<"Local "<<score[0]<<" "<<score[1]<<"/////////////////"<<endl;
        return {score[0],score[1]};
    }
    inline float get_score(int len,int block,int empty=0)
    {
        if(len==-1)
            return -1;
        float s=0;
        if(len>=5)
        {
            s=SCORE_5;
//            if(empty)
//            {
//                s=1000;
//                if(block)
//                    s=100;
//            }
        }
        else
        {
            s=pow(10,len-1-block);
            if(block>=2)
                s=0;
//            if(empty)
//                s*=0.5;
        }
        return s;
    }
    vector<float> Score_local_reverse(qz &temp,bool set=1,bool print=0)
    {
        //计算场上双方的分数
        bool flagBoard[N][N][4]={0};
        float score[2]={0};

            //temp.print();
            int I=temp.i,J=temp.j,P=temp.P;
            int neg_P=!(P-1)+1;
            for(int drc=0;drc<=3;drc++)
            {
                I=temp.i;J=temp.j;
//                if(flagBoard[I][J][drc])
//                    continue;
                changeIJ(I,J,drc);
                bool oneP=0,twoP=0;
                if(IJinBoard(I,J))
                {
                if(board[I][J].P==P)
                {
                    oneP=1;
                    vector<int> tmp=calMaxLen(I,J,P,flagBoard,drc);
                    int len=tmp[0],block=tmp[1],empty=tmp[2],I0=tmp[3],J0=tmp[4],I1=tmp[5],J1=tmp[6];
                    float s=get_score(len,block,empty);
                    if(s>0)
                    {
                        if(I0==I&&J0==J)
                        {
                            score[P-1]-=board[I1][J1].score[drc];

                        }
                        else
                        {
                            score[P-1]-=board[I0][J0].score[drc];
                        }
                        score[P-1]+=s;
                        if(set)
                        {
                        board[I0][J0].set1(drc,s,block,I1,J1);board[I1][J1].set1(drc,s,block,I0,J0);
                        }
                    }
                }
                else if(board[I][J].P)
                {

                    qz &neg_q=board[I][J];
                    int len=neg_q.getlen(drc),block=neg_q.block[drc]-1;
                    float s=get_score(len,block);
                    score[neg_P-1]-=neg_q.score[drc]-s;
                    int another_i=neg_q.another_i[drc],another_j=neg_q.another_j[drc];
                    if(set)
                    {
                    board[another_i][another_j].set1(drc,s,block);
                    board[I][J].set1(drc,s,block);
                    }
                }
                }
                I=temp.i;J=temp.j;
                changeIJ(I,J,drc,1);
                if(IJinBoard(I,J))
                {
                if(board[I][J].P==P)
                {
                    twoP=1;
                    vector<int> tmp=calMaxLen(I,J,P,flagBoard,drc);
                    int len=tmp[0],block=tmp[1],empty=tmp[2],I0=tmp[3],J0=tmp[4],I1=tmp[5],J1=tmp[6];
                    float s=get_score(len,block,empty);
                    if(s>0)
                    {
                        if(!oneP)
                        {
                        if(I0==I&&J0==J)
                        {
                            score[P-1]-=board[I1][J1].score[drc];

                        }
                        else
                        {
                            score[P-1]-=board[I0][J0].score[drc];
                        }
                        }
                        score[P-1]+=s;
                        if(set)
                        {
                        board[I0][J0].set1(drc,s,block,I1,J1);board[I1][J1].set1(drc,s,block,I0,J0);
                        }
                    }
                }
                else if(board[I][J].P)
                {
                    qz &neg_q=board[I][J];
                    int len=neg_q.getlen(drc),block=neg_q.block[drc]-1;
                    float s=get_score(len,block);
                    score[neg_P-1]-=neg_q.score[drc]-s;
                    int another_i=neg_q.another_i[drc],another_j=neg_q.another_j[drc];
                    if(set)
                    {
                    board[another_i][another_j].set1(drc,s,block);
                    board[I][J].set1(drc,s,block);
                    }
                }
                }
                I=temp.i;J=temp.j;
                if(!oneP&&!twoP)
                {
                    vector<int> tmp=calMaxLen(I,J,P,flagBoard,drc);
                    int len=tmp[0],block=tmp[1],empty=tmp[2],I0=tmp[3],J0=tmp[4],I1=tmp[5],J1=tmp[6];
                    float s=get_score(len,block,empty);
                    score[P-1]-=s;
                }


//                if(print&&len>1)
//                    cout<<"Local drc:"<<drc<<" len:"<<len<<" block:"<<block<<" empty:"<<empty<<" P:"<<P<<" s:"<<s<<" I:"<<I<<" J:"<<J<<endl;
//                if(len>1)
//                cout<<I0<<" "<<J0<<" "<<I1<<" "<<J1<<" "<<I<<" "<<J<<endl;

            }
            flagBoard[I][J][0]=1;flagBoard[I][J][1]=1;flagBoard[I][J][2]=1;flagBoard[I][J][3]=1;

        if(print)
            cout<<"Local Reverse:"<<score[0]<<" "<<score[1]<<"/////////////////"<<endl;
        return {score[0],score[1]};
    }
    void print(vector<qz> &QZs,bool _score=0,bool _block=0,bool _anotherIJ=0)
    {
        for(int i=0;i<QZs.size();i++)
        {
            int I=QZs[i].i,J=QZs[i].j;
            board[I][J].print(_score,_block,_anotherIJ);
        }
    }

};
vector<int> calMaxLen(int I,int J,int P,int board[N][N],bool flagBoard[N][N][4],int drc)
{

    //P=1或2
    int i=I,j=J;
    flagBoard[i][j][drc]=1;
    changeIJ(i,j,drc);
    int len=1,block=0,empty=0;
    while(i>=0&&j>=0&&i<N&&j<N)
    {
        if(flagBoard[i][j][drc]&&board[i][j]==P)
        {
            return {-1,-1};
        }
        if(!board[i][j])
        {
            if(!empty)
            {
                int i1=i,j1=j;
                changeIJ(i1,j1,drc);
                if(IJinBoard(i1,j1)&&board[i1][j1]==P)
                {
                    empty++;
                    i=i1;
                    j=j1;
                }
                else
                    break;
            }
            else
                break;
        }
        if(board[i][j]!=P)
        {
            block++;
            break;
        }

        len++;
        if(board[i][j])
            flagBoard[i][j][drc]=1;
        changeIJ(i,j,drc);
    }
    if(!(i>=0&&j>=0&&i<N&&j<N))
    {
        block++;
    }
    i=I;j=J;
    changeIJ(i,j,drc,1);
    while(i>=0&&j>=0&&i<N&&j<N)
    {
        if(flagBoard[i][j][drc]&&board[i][j]==P)
        {
            return {-1,-1};
        }
        if(!board[i][j])
        {
            if(!empty)
            {
                int i1=i,j1=j;
                changeIJ(i1,j1,drc,1);
                if(IJinBoard(i1,j1)&&board[i1][j1]==P)
                {
                    empty++;
                    i=i1;
                    j=j1;
                }
                else
                    break;
            }
            else
                break;
        }
        if(board[i][j]!=P)
        {
            block++;
            break;
        }
        len++;
        if(board[i][j])
            flagBoard[i][j][drc]=1;
        changeIJ(i,j,drc,1);
    }
    if(!(i>=0&&j>=0&&i<N&&j<N))
    {
        block++;
    }
    return {len,block,empty};
}
vector<float> Score(int board[N][N],vector<qz> &QZs,bool print=0)
{
    //计算场上双方的分数
    bool flagBoard[N][N][4]={0};
    float score[2]={0};
    for(int i=0;i<QZs.size();i++)
    {
        qz &temp=QZs[i];
        int I=temp.i,J=temp.j,P=temp.P;
        for(int drc=0;drc<=3;drc++)
        {
            if(flagBoard[I][J][drc])
                continue;
            vector<int> tmp=calMaxLen(I,J,P,board,flagBoard,drc);
            int len=tmp[0],block=tmp[1],empty=tmp[2];

            if(len==-1)
                continue;
            float s=0;
            if(len>=5)
            {
                s=10000000;
                if(empty)
                {
                    s=1000;
                    if(block)
                        s=100;
                }
            }
            else
            {
//                if(empty&&len<3)
//                    continue;
                s=pow(10,len-1-block);
                if(block>=2)
                    s=0;
                if(empty)
                    s*=0.5;
            }

            if(print&&len>1)
                cout<<"drc:"<<drc<<" len:"<<len<<" block:"<<block<<" empty:"<<empty<<" P:"<<P<<" s:"<<s<<" I:"<<I<<" J:"<<J<<endl;
            score[P-1]+=s;
        }
        flagBoard[I][J][0]=1;flagBoard[I][J][1]=1;flagBoard[I][J][2]=1;flagBoard[I][J][3]=1;
    }
    if(print)
        cout<<score[0]<<" "<<score[1]<<"/////////////////"<<endl;
    return {score[0],score[1]};
}
vector<float> decision(int board[N][N],vector<qz> &QZs,int P,int type=0)
{
    //P=0或1
    qz temp;
    QZs.push_back(temp);
    float max_myScore,max_opScore;
    float max_score=-100000000,min_score=100000000;
    int max_i=0,max_j=0;
    float final_score_AI_maxmin=0;
    for(int i=0;i<N;i++)
        for(int j=0;j<N;j++)
        {
            if(board[i][j])
                continue;
            board[i][j]=P+1;
            QZs[QZs.size()-1].set(i,j,P+1);

            vector<float> score=Score(board,QZs);
            float myScore=score[P],opScore=score[!P];
            float AIScore=myScore,HumanScore=opScore;
            if(P!=MainWindow::P_AI)
            {
                AIScore=opScore;
                HumanScore=myScore;
            }
            if(AIfirst)
                AIScore*=1.1;
            else
                HumanScore*=1.1;
            float final_score_AI=AIScore-HumanScore;
//            float final_score=myScore;
            if(type)
            {
                if(myScore>max_score)
                {
                    max_score=myScore;
                    max_i=i;
                    max_j=j;
                    max_myScore=max_score;
                    max_opScore=opScore;
                }
            }
            else
            {
            if(MainWindow::P_AI==P&&final_score_AI>max_score)
            {
                max_score=final_score_AI;
                max_i=i;
                max_j=j;
                max_myScore=myScore;
                max_opScore=opScore;
                final_score_AI_maxmin=final_score_AI;
            }
            else if(MainWindow::P_AI!=P&&final_score_AI<min_score)
            {
                min_score=final_score_AI;
                max_i=i;
                max_j=j;
                max_myScore=myScore;
                max_opScore=opScore;
                final_score_AI_maxmin=final_score_AI;
            }
            }
            board[i][j]=0;
        }
    QZs.pop_back();
    return {float(max_i),float(max_j),max_myScore,max_opScore,final_score_AI_maxmin};

}
inline float genAIScore(float s0,float s1)
{
    float AIScore=s0,HumanScore=s1;
    if(MainWindow::P_AI!=0)
    {
        AIScore=s1;
        HumanScore=s0;
    }
    if(D%2)
        HumanScore*=1.2;
    else
        AIScore*=1.2;
//    if(AIfirst)
//        AIScore*=1.1;
//    else
//        HumanScore*=1.1;
    return AIScore-HumanScore;
}
inline int clearChildNodes(vector<Node*> &Nodes,Node* par)
{
    Node *NodetobeClear=Nodes.back();
    int cut_N=0;
    while(NodetobeClear->parent==par)
    {
        cut_N++;
        Nodes.pop_back();
        delete NodetobeClear;
        if(!Nodes.empty())
            NodetobeClear=Nodes.back();
        else
            break;
    }
    return cut_N;
}
vector<int> alpha_beta(vector<Node*> &Nodes,int board[N][N],vector<qz> &QZs)
{
    Zobrist zobrist;
    int total_N=0,cut_N=0;
    long long crt_hashkey=0;
    unordered_map<string,float> test_map;
    while(!Nodes.empty())
    {

        Node *temp=Nodes.back();
        //Nodes.pop_back();
        int I0=temp->i,J0=temp->j,P=temp->P,d=temp->d;
        long long hashkey;
        if(d)
        {
            hashkey=zobrist.get_hashkey(I0,J0,P);
        }
        //cout<<"I0 "<<I0<<" J0 "<<J0<<" d "<<d<<" score "<<temp->score<<" P "<<P<<" visited "<<temp->visited<<endl;
        if(temp->visited)
        {
            if(!d)
            {
                cout<<"total_N "<<total_N<<" cut_N "<<cut_N<<endl;
                return {temp->i,temp->j};
            }
            temp->setScore();
            if(d>2)
            if(!zobrist.set_value(crt_hashkey,d,temp->score,1))
                cout<<"no"<<endl;
            QZs.pop_back();
            board[I0][J0]=0;
            Nodes.pop_back();
            crt_hashkey^=hashkey;
            if(temp->setParAlphaBeta())
            {
                //父节点的alpha>=beta，清空属于该父节点的本层节点
//                Node *NodetobeClear=Nodes.back(),*par=temp->parent;
//                while(NodetobeClear->parent==par)
//                {
//                    cut_N++;
//                    Nodes.pop_back();
//                    if(!Nodes.empty())
//                        NodetobeClear=Nodes.back();
//                    else
//                        break;
//                }
                cut_N+=clearChildNodes(Nodes,temp->parent);
            }
            if(temp->ifDelete)
                delete temp;
            continue;
        }
        else
            temp->visited=1;
        if(I0!=-1)
        {
            crt_hashkey^=hashkey;
            board[I0][J0]=P+1;
            QZs.push_back(qz(I0,J0,P+1));
            float tmpScore=zobrist.get_value(crt_hashkey,d).score;
            if(d>2&&tmpScore)
            {
                //TEST
                if(zorist_test)
                {
                string tmpS="";
                for(int i=2;i>=0;i--)
                {
                    tmpS+=to_string(QZs[QZs.size()-i-1].i)+to_string(QZs[QZs.size()-i-1].j);
                }
                string tmpS1=tmpS.substr(4,2)+tmpS.substr(2,2)+tmpS.substr(0,2);
                if(test_map[tmpS1]!=tmpScore&&test_map[tmpS]!=tmpScore)
                {
                    cout<<tmpS1<<" "<<zobrist.test_map[crt_hashkey]<<endl;
                    cout<<test_map[tmpS1]<<" "<<test_map[tmpS]<<" "<<tmpScore<<endl;
                }
                temp->setScore(tmpScore);
                }
                //TEST
                QZs.pop_back();
                board[I0][J0]=0;
                Nodes.pop_back();
                crt_hashkey^=hashkey;
                if(temp->setParAlphaBeta())
                {
                    //父节点的alpha>=beta，清空属于该父节点的本层节点
                    cut_N+=clearChildNodes(Nodes,temp->parent);

                }
                if(temp->ifDelete)
                    delete temp;
                continue;
            }
        }
        if(d<D)
        {
            if(temp->parent)
                temp->setAlphaBetafromPar();
            for(int i=max(0,Imin-D);i<min(N,Imax+D);i++)
                for(int j=max(0,Jmin-D);j<min(N,Jmax+D);j++)
                {
                    if(board[i][j])
                        continue;
                    total_N++;
                    Node *child=new Node(i,j,d+1,!P,temp);
                    child->setAlphaBetafromPar();
                    Nodes.push_back(child);
                }
        }
        else
        {
            //print(QZs);
            vector<float> score=Score(board,QZs);
            float AIScore=genAIScore(score[0],score[1]);
            temp->score=AIScore;temp->fixed=1;
            //TEST
            string tmpS="";
            if(zorist_test)
            {
            for(int i=0;i<3;i++)
            {
                tmpS+=to_string(QZs[QZs.size()-i-1].i)+to_string(QZs[QZs.size()-i-1].j);
            }
            test_map[tmpS]=AIScore;
            }
            //TEST
//            if(d>2)
//                if(!zobrist.set_value(crt_hashkey,d,AIScore,tmpS))
//                    cout<<"no"<<endl;

            //cout<<AIScore<<endl;
            QZs.pop_back();
            board[I0][J0]=0;
            Nodes.pop_back();
            crt_hashkey^=hashkey;
            if(temp->setParAlphaBeta())
            {
                //父节点的alpha>=beta，清空属于该父节点的本层节点
                cut_N+=clearChildNodes(Nodes,temp->parent);
            }
            if(temp->ifDelete)
                delete temp;
        }
    }
}
int recur_D=4;
struct recur_return{
    float score=0;
    vector<int> path;
};
bool recur_reverse=0;
inline float genAIScore_recur(float s0,float s1)
{
    float AIScore=s0,HumanScore=s1;
    if(MainWindow::P_AI!=0)
    {
        AIScore=s1;
        HumanScore=s0;
    }
    if((recur_D%2)^recur_reverse)
        HumanScore*=1.2;
    else
        AIScore*=1.2;
//    if(AIfirst)
//        AIScore*=1.1;
//    else
//        HumanScore*=1.1;
    return AIScore-HumanScore;
}
bool jianzhi=1;
IntelligentBoard IBoard;
recur_return alpha_beta_recur(float alpha,float beta,qz q,int d,vector<qz> &QZs,float SCORE0,float SCORE1)
{
    int I0=q.i,J0=q.j,P=q.P-1;//P=0,1
    recur_return max_r;
    float delta_s0=0,delta_s1=0;
    vector<qz> editedQZs;
    //cout<<"I0:"<<I0<<",J0:"<<J0<<",d:"<<d<<endl;
    if(I0!=-1)
    {
//        hashkey^=zobrist.get_hashkey(I0,J0,P);
//        if(d>=3)
//        {
//        ZobristNode zobristnode=zobrist.get_value(hashkey,d);
//        if(zobristnode.score)
//        {
//            if(zobristnode.fixed)
//            {
//                max_r.score=zobristnode.score;
//                return max_r;
//            }
//            else
//                alpha=max(alpha,zobristnode.score);
//        }
//        }
        IBoard.board[I0][J0].P=P+1;
        QZs.push_back(q);
        vector<float> deltas;
        bool last_d=0;
        if(d==recur_D)
        {
            last_d=1;
        }
    //    vector<float> trueScore=IBoard.Score(QZs,0,0);
    //    if(fabs(trueScore[0]-SCORE0-delta_s0)>0.01||fabs(trueScore[1]-SCORE1-delta_s1)>0.01)
    //    {
    //        cout<<"FAULT!!!!!!!!!!"<<endl;
    //        cout<<delta_s0<<" "<<delta_s1<<endl;
    //        //IBoard.print(QZs,1);
    //    }
        deltas=IBoard.Score_local(QZs,QZs.size()-1,editedQZs,!last_d);
        delta_s0=deltas[0];delta_s1=deltas[1];
        if(SCORE0+delta_s0>=SCORE_5||SCORE1+delta_s1>=SCORE_5)
        {
            if(!last_d)
                IBoard.setQZfromQZs(editedQZs);
            IBoard.board[I0][J0].reset();
            QZs.pop_back();
            float AIScore=genAIScore_recur(SCORE0+delta_s0,SCORE1+delta_s1);
            if((d%2)^recur_reverse)
                AIScore*=-1;
            max_r.score=AIScore;
//            if(d>=3)
//            zobrist.set_value(hashkey,d,AIScore,1);
            return max_r;
        }
    }
    if(d<recur_D)
    {
        float maxScore=-100000000;
        int max_i,max_j;
        vector<int> setCandidateQZs;
        if(I0!=-1)
            setCandidateQZs=IBoard.setCandidateQZ(I0,J0);
        for(int i=max(0,Imin-recur_D);i<min(N,Imax+recur_D);i++)
            for(int j=max(0,Jmin-recur_D);j<min(N,Jmax+recur_D);j++)
            {
                if(IBoard.board[i][j].P)
                    continue;
                if(!IBoard.candidateQZ[i][j])
                    continue;
                qz next_q(i,j,!P+1);
                recur_return r=alpha_beta_recur(-beta,-alpha,next_q,d+1,QZs,SCORE0+delta_s0,SCORE1+delta_s1);
                float v=r.score;
                v*=-1;
                if(v>maxScore)
                {
                    maxScore=v;
                    max_i=i;
                    max_j=j;
                    max_r.path=r.path;
                }
                alpha=max(maxScore,alpha);
                if(v>=beta)
                {
                    IBoard.setQZfromQZs(editedQZs);
                    IBoard.board[I0][J0].reset();
                    QZs.pop_back();
                    r.score=v;
                    //r.path.push_back(qz(max_i,max_j,P+1));
//                    if(d>=3)
//                    zobrist.set_value(hashkey,d,v,0);
                    IBoard.setCandidateQZ_reverse(setCandidateQZs);
                    return r;
                }
                //total_N++;
            }

//        cout<<I0<<" "<<J0<<" "<<max_i<<" "<<max_j<<" "<<maxScore<<" "<<d<<endl;
//        for(int i=0;i<QZs.size();i++)
//            cout<<QZs[i].i<<" "<<QZs[i].j<<" "<<QZs[i].P<<";";
//        cout<<endl;
        if(I0!=-1)
        {
            IBoard.setQZfromQZs(editedQZs);
        IBoard.board[I0][J0].reset();
        QZs.pop_back();
        IBoard.setCandidateQZ_reverse(setCandidateQZs);
        }
        max_r.score=maxScore;
        max_r.path.push_back(max_i);max_r.path.push_back(max_j);
//        if(d>=3)
//        zobrist.set_value(hashkey,d,maxScore,1);
        //cout<<maxScore<<endl;
        return max_r;
    }
    else
    {
        float AIScore=genAIScore_recur(SCORE0+delta_s0,SCORE1+delta_s1);
        IBoard.board[I0][J0].P=0;
        QZs.pop_back();
        if((recur_D%2)^recur_reverse)
            AIScore*=-1;
        max_r.score=AIScore;
//        if(d>=3)
//        zobrist.set_value(hashkey,d,AIScore,1);
        //cout<<AIScore<<endl;
        return max_r;
    }
    return max_r;
}
recur_return alpha_beta_recur_origin(float alpha,float beta,qz q,int d,vector<qz> &QZs)
{
    int I0=q.i,J0=q.j,P=q.P-1;//P=0,1
    recur_return max_r;
    if(I0!=-1)
    {
    IBoard.board[I0][J0].P=P+1;
    QZs.push_back(q);
    }
    if(d<recur_D)
    {
        float maxScore=-100000000;
        int max_i,max_j;
        for(int i=max(0,Imin-recur_D);i<min(N,Imax+recur_D);i++)
            for(int j=max(0,Jmin-recur_D);j<min(N,Jmax+recur_D);j++)
            {
                if(IBoard.board[i][j].P)
                    continue;
                qz next_q(i,j,!P+1);
                recur_return r=alpha_beta_recur_origin(-beta,-alpha,next_q,d+1,QZs);
                float v=r.score;
                v*=-1;
                if(v>maxScore)
                {
                    maxScore=v;
                    max_i=i;
                    max_j=j;
                    max_r.path=r.path;
                }
                alpha=max(maxScore,alpha);
//                if(v>=beta)
//                {
//                    IBoard.board[I0][J0].P=0;
//                    QZs.pop_back();
//                    r.score=v;
//                    //r.path.push_back(qz(max_i,max_j,P+1));
//                    return r;
//                }
                //total_N++;
            }

//        cout<<I0<<" "<<J0<<" "<<max_i<<" "<<max_j<<" "<<maxScore<<" "<<d<<endl;
//        for(int i=0;i<QZs.size();i++)
//            cout<<QZs[i].i<<" "<<QZs[i].j<<" "<<QZs[i].P<<";";
//        cout<<endl;
        if(I0!=-1)
        {
        IBoard.board[I0][J0].P=0;
        QZs.pop_back();
        }
        max_r.score=maxScore;
        max_r.path.push_back(max_i);max_r.path.push_back(max_j);
        return max_r;
    }
    else
    {
        vector<float> score=IBoard.Score(QZs,0,0);
        float AIScore=genAIScore_recur(score[0],score[1]);
        IBoard.board[I0][J0].P=0;
        QZs.pop_back();
        if((recur_D%2)^recur_reverse)
            AIScore*=-1;
        max_r.score=AIScore;
        return max_r;
    }
    return max_r;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    CRandomMersenne RanGen(time(NULL));
    for(int i=0;i<N;i++)
        for(int j=0;j<N;j++)
        {
            zobrist0[i][j]=get_rand64(RanGen);
            zobrist1[i][j]=get_rand64(RanGen);
        }
    for(int i=0;i<N;i++)
        for(int j=0;j<N;j++)
        {
            IBoard.board[i][j].set(i,j,0);
        }
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::paintEvent(QPaintEvent *e)
{
    QPainter *pa;
    pa=new QPainter();
    pa->begin(this);
    QPen pen(Qt::black,2);
//     pen.setStyle(Qt::DashDotDotLine);
//     pen.setWidth(1);

     pa->setPen(pen);
//     pa->translate(0, 0);
     QVector<QLineF>lines;
     int SCREEN_W=rowWid*N;
     for(int i = 0; i < N; ++i)
         {
            QPointF hlinestart(0,rowWid*(i+1));
            QPointF hlinesend(SCREEN_W,rowWid*(i+1));//宽
            QLineF lineH(hlinestart,hlinesend);
            QPointF vlinestart(rowWid*(i+1),0);
            QPointF vlinesend(rowWid*(i+1),SCREEN_W);//高
            QLineF lineV(vlinestart,vlinesend);
            lines.push_back(lineH);
            lines.push_back(lineV);
         }
     pa->drawLines(lines);
     //画棋子
     pa->setPen(QPen(Qt::blue,0));//设置画笔形式
     const int padding=10;
     for(int i=0;i<N;i++)
         for(int j=0;j<N;j++)
            if(this->board[i][j])
            {
                if(this->board[i][j]==1)
                {
                    pa->setBrush(QColor(Qt::black));//设置画刷，如果不画实现的直接把Brush设置为setBrush(Qt::NoBrush);
                }
                else
                {
                    pa->setBrush(QColor(Qt::red));//设置画刷，如果不画实现的直接把Brush设置为setBrush(Qt::NoBrush);
                }
                pa->drawEllipse(j*rowWid+padding/2,i*rowWid+padding/2,rowWid-padding,rowWid-padding);//画圆
            }
     if(alpha_beta_test)
     {
         for(int k=0;k<min(predict_paint_N,int(predict_QZs.size()));k++)
         {
             int i=predict_QZs[k].i,j=predict_QZs[k].j;
             if(k%2)
                 pa->setBrush(QColor(255,0,0,140));
             else
                 pa->setBrush(QColor(60,60,60,140));
             pa->drawEllipse(j*rowWid+padding/2,i*rowWid+padding/2,rowWid-padding,rowWid-padding);
         }
         for(int k=0;k<min(secondery_paint_N,int(secondery_predict.size()));k++)
         {
             int i=secondery_predict[k].i,j=secondery_predict[k].j;
             if(secondery_predict[k].P==2)
                 pa->setBrush(QColor(255,0,0,70));
             else
                 pa->setBrush(QColor(60,60,60,70));
             pa->drawEllipse(j*rowWid+padding/2,i*rowWid+padding/2,rowWid-padding,rowWid-padding);
         }
     }
    pa->end();
    delete pa;
}
void MainWindow::mousePressEvent(QMouseEvent *m){
    int mx=m->x(),my=m->y();
    int J=mx/rowWid,I=my/rowWid;
    bool score_print=1;
//    cout<<I<<" "<<J<<endl;
    if(I>=N||J>=N)
        return ;

    // ///////////for test//////////////////
    float true_ds0,true_ds1,predict_ds0,predict_ds1;
    bool leftbutton=0;
    // ///////////for test//////////////////

    if(m->button()==Qt::LeftButton)
    {
        leftbutton=1;
    if(this->board[I][J])
        return ;
    Imin=min(Imin,I);Jmin=min(Jmin,J);Imax=max(Imax,I);Jmax=max(Jmax,J);
    this->board[I][J]=crtP+1;
    IBoard.board[I][J].P=crtP+1;
    qz temp;
    temp.set(I,J,crtP+1);
    this->QZs.push_back(temp);
    this->AI2();
    crtP=!crtP;
    }
    bool qz_del=0;
    if(m->button()==Qt::RightButton)
    {
        score_print=1;
        if(this->board[I][J])
        {
            qz_del=1;
            if(this->board[I][J]==!P_AI+1)
            {

            this->board[I][J]=0;
                qz temp=IBoard.board[I][J];
//                IBoard.board[I][J].P=0;
                IBoard.board[I][J].reset();
                int idx=findqz(I,J);
                this->QZs.erase(this->QZs.begin()+idx);
                vector<float> ds=IBoard.Score_local_reverse(temp,1,1);
                predict_ds0=ds[0];
                predict_ds1=ds[1];
//                IBoard.setQZfromQZs(this->editedQZs);
            }
        }
        else
        {
            this->board[I][J]=!P_AI+1;
            IBoard.board[I][J].P=!P_AI+1;
            this->QZs.push_back(qz(I,J,!P_AI+1));
        }
    }
    if(m->button()==Qt::MiddleButton)
    {
        score_print=1;
        if(this->board[I][J])
        {
            qz_del=1;
            if(this->board[I][J]==P_AI+1)
            {

            this->board[I][J]=0;
                qz temp=IBoard.board[I][J];
//                IBoard.board[I][J].P=0;
                IBoard.board[I][J].reset();
            int idx=findqz(I,J);
            this->QZs.erase(this->QZs.begin()+idx);
            vector<float> ds=IBoard.Score_local_reverse(temp,1,1);
            predict_ds0=ds[0];
            predict_ds1=ds[1];
//            IBoard.setQZfromQZs(this->editedQZs);
            }
        }
        else
        {
            this->board[I][J]=P_AI+1;
            IBoard.board[I][J].P=P_AI+1;
            this->QZs.push_back(qz(I,J,P_AI+1));
        }
    }
    this->repaint();
    //vector<float> score=Score(this->board,this->QZs,score_print);
    vector<float> score=IBoard.Score(this->QZs,score_print,0);
    true_ds0=score[0]-last_s0;
    true_ds1=score[1]-last_s1;
    last_s0=score[0];
    last_s1=score[1];
    if(!qz_del&&!leftbutton)
    {
    vector<float> ds=IBoard.Score_local(this->QZs,this->QZs.size()-1,this->editedQZs,1,1);
    predict_ds0=ds[0];
    predict_ds1=ds[1];
    }

    //IBoard.print(this->QZs,1);
    P1_SCORE=score[0];P2_SCORE=score[1];PREHENSIVE_SCORE=genAIScore_recur(P1_SCORE,P2_SCORE);
    string labeltext="黑方："+to_string(P1_SCORE)+"\n红方："+to_string(P2_SCORE)+"\n综合得分："+to_string(PREHENSIVE_SCORE);
    if(!leftbutton&&(fabs(predict_ds0-true_ds0)>0.01||fabs(predict_ds1-true_ds1)>0.01))
    {
        cout<<"ERROR!!!!!!!!!!"<<fabs(predict_ds0-true_ds0)<<" "<<fabs(predict_ds1-true_ds1)<<endl;
        labeltext+="\nERROR!!!!";
    }
    ui->label->setText(labeltext.c_str());

}

void MainWindow::AI(){
    qz temp;
    int type=0;
    this->QZs.push_back(temp);
    float max_score=-100000000,min_score=100000000;
    int max_i=-1,max_j=-1;
    for(int i=0;i<N;i++)
        for(int j=0;j<N;j++)
        {
            if(this->board[i][j])
                continue;
            this->board[i][j]=P_AI+1;
            this->QZs[QZs.size()-1].set(i,j,P_AI+1);
            vector<float> DCS=decision(this->board,this->QZs,!P_AI,type);

//            vector<float> score=Score(this->board,this->QZs);
            float AI_score=DCS[3],human_score=DCS[2];

            if(!type)
            {
            float AI_score=DCS[4];
            if(AI_score>max_score)
            {
                max_score=AI_score;
                max_i=i;
                max_j=j;
            }
            }
            else
            {
                float final_score;
                if(AIfirst)
                    final_score=AI_score*1.1-human_score;
                else
                    final_score=AI_score-human_score*1.1;
                if(final_score>max_score)
                {
                    max_score=final_score;
                    max_i=i;
                    max_j=j;
                }
            }
            this->board[i][j]=0;
        }
    this->board[max_i][max_j]=P_AI+1;
    this->QZs[QZs.size()-1].set(max_i,max_j,P_AI+1);
    crtP=!crtP;
    this->repaint();
}
void MainWindow::AI1(){
    Node *Node0=new Node(-1,-1,0,!P_AI);
    vector<Node*> Nodes;
    Nodes.push_back(Node0);
    vector<int> ans=alpha_beta(Nodes,this->board,this->QZs);
    if(alpha_beta_test)
    {
        predict_paint_N=0;
        predict_QZs.clear();
    Node *Node1=Node0->best_child->best_child;
    for(int d=1;d<D;d++)
    {
        cout<<"predict next ij:"<<Node1->i<<" "<<Node1->j<<" SCORE"<<Node1->score<<endl;
        predict_QZs.push_back(qz(Node1->i,Node1->j,Node1->P));
        Node1=Node1->best_child;
    }
    }
    int I=ans[0],J=ans[1];
    Imin=min(Imin,I);Jmin=min(Jmin,J);Imax=max(Imax,I);Jmax=max(Jmax,J);
    this->board[I][J]=P_AI+1;
    this->QZs.push_back(qz(I,J,P_AI+1));
    crtP=!crtP;
    this->repaint();
}
void MainWindow::AI2(){
    secondery_paint_N=0;
    IBoard.clearCandidate();
    vector<float> tempS=IBoard.Score(this->QZs,0,1,0,1);
    IBoard.printCandidate();
    float SCORE0=tempS[0],SCORE1=tempS[1];
    int I,J;
    int origin_recurD=recur_D;
    if(this->QZs.size()==1)
    {
        recur_D=1;
    }
    recur_return r=alpha_beta_recur(-100000000,100000000,qz(-1,-1,1),0,this->QZs,SCORE0,SCORE1);
    IBoard.printCandidate();
    if(this->QZs.size()==1)
    {
        recur_D=origin_recurD;
    }
    vector<int> path=r.path;
    cout<<"AI2:r.score "<<r.score<<endl;
//    cout<<"AI2:path.size() "<<path.size()<<endl;
    predict_QZs.clear();
    predict_paint_N=0;
    int tempP=!P_AI;
    for(int d=path.size()-4;d>=0;d-=2)
    {
        cout<<"predict next ij:"<<path[d]<<" "<<path[d+1]<<endl;
        predict_QZs.push_back(qz(path[d],path[d+1],tempP+1));
        tempP=!tempP;
    }
    I=path[path.size()-2];J=path[path.size()-1];
    Imin=min(Imin,I);Jmin=min(Jmin,J);Imax=max(Imax,I);Jmax=max(Jmax,J);
    this->board[I][J]=P_AI+1;
    IBoard.board[I][J].P=P_AI+1;
    this->QZs.push_back(qz(I,J,P_AI+1));
    crtP=!crtP;
    this->repaint();
}

void MainWindow::on_pushButton_clicked(){
    zobrist.clear();
    AIfirst=!AIfirst;
    memset(this->board,0,sizeof(this->board));
    this->QZs.clear();
    Imin=N;Imax=-1;Jmin=N;Jmax=-1;
    if(AIfirst)
    {
        //cout<<1<<endl;
        this->board[N/2][N/2]=P_AI+1;
        IBoard.board[N/2][N/2].P=P_AI+1;
        Imin=N/2;Imax=N/2;Jmin=N/2;Jmax=N/2;
        this->QZs.push_back(qz(N/2,N/2,P_AI+1));

    }
    crtP=!P_AI;
    this->repaint();
}
void MainWindow::on_pushButton_2_clicked(){
    predict_paint_N++;
    this->repaint();
}
void MainWindow::on_pushButton_3_clicked(){
    jianzhi=0;
    recur_D--;
    secondery_paint_N=recur_D;
    recur_reverse=1;
    vector<float> tempS=IBoard.Score(this->QZs,0,1,0,1);
    float SCORE0=tempS[0],SCORE1=tempS[1];
    recur_return r=alpha_beta_recur(-100000000,100000000,qz(-1,-1,2),0,this->QZs,SCORE0,SCORE1);
    vector<int> path=r.path;
    cout<<r.score<<endl;
//    cout<<path.size()<<endl;
    secondery_predict.clear();
    int tempP=!P_AI;
    for(int d=path.size()-2;d>max(int(path.size()-1-recur_D),-1);d-=2)
    {
        cout<<"predict next ij:"<<path[d]<<" "<<path[d+1]<<endl;
        predict_QZs.push_back(qz(path[d],path[d+1],tempP+1));
        tempP=!tempP;
    }
    jianzhi=1;
    recur_D++;
    recur_reverse=0;
    this->repaint();
}
void MainWindow::on_pushButton_4_clicked(){
//    jianzhi=0;
    secondery_paint_N=recur_D;
//    vector<float> tempS=IBoard.Score(this->QZs);
//    float SCORE0=tempS[0],SCORE1=tempS[1];
    recur_return r=alpha_beta_recur_origin(-100000000,100000000,qz(-1,-1,1),0,this->QZs);
    vector<int> path=r.path;
    cout<<"button4:r.score "<<r.score<<endl;
//    cout<<path.size()<<endl;
    secondery_predict.clear();
    int tempP=P_AI;
    for(int d=path.size()-2;d>max(int(path.size()-1-recur_D),-1);d-=2)
    {
        cout<<"secondery predict next ij:"<<path[d]<<" "<<path[d+1]<<endl;
        secondery_predict.push_back(qz(path[d],path[d+1],tempP+1));
        tempP=!tempP;
    }
//    jianzhi=1;
    this->repaint();
}
