class Player
{
    protected:
    int tid,size;
    int *vector_timer, *vector_modes;
    int lamport_timer;

    int ready; //(mode==0) ready4fight || ready4help (mode==1)
    int yes;
    int no;

    public:
    Player(int tid, int size);

    virtual void tmp()=0;
};

class Veteran :public Player
{
    int tid_sparring_partner;
    int tid_supp1,tid_supp2;

    public:
    Veteran(int tid, int size);
    void tmp(){;}
};

class Support :public Player
{
    int tid_veteran;

    public:
    Support(int tid, int size);
    void tmp(){;}
};