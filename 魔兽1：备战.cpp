#include<bits/stdc++.h>
using namespace std;


struct deleter {
	template<typename T>
	void operator()(const T* ptr) const { delete ptr; }
};

////////////////////////////////全局变量////////////////////////////////


enum Color {red=0,blue=1 };//颜色阵营枚举
enum Kind {dragon=0,ninja=1,iceman=2,lion=3,wolf=4};//武士种类枚举
string colorstr[2]={"red","blue"};
string kindstr[5]={"dragon","ninja","iceman","lion","wolf"};



int healthmap[5]={0};//生命值初始化
int damagemap[5]={0};//攻击力初始化
int LIFE_ELEMENT=0;//生命元
bool redstop=false;
bool bluestop=false;

Kind create_chain[2][5]={{iceman,lion,wolf,ninja,dragon},{lion,dragon,ninja,iceman,wolf}};//制造顺序
int op_create_chain[2][5]={{4,3,0,1,2},{1,2,3,0,4}};//顺序逆映射


////////////////////////////////////////////////////////////////////////


////////////////////////////////设计武士/////////////////////////////////
class Solider{
    //武士类
public:
    //公有接口
    int get_id(){return id;}
    int get_health(){return health;}
    int get_damage(){return damage;}
    Kind get_kind(){return kind;}
protected:
    //受保护的构造函数，无法直接调用，防止基类对象实例化
    Solider(int _id,int _health,int _damage):id(_id),health(_health),damage(_damage){}
    Solider(const Solider& s):id(s.id),health(s.health),damage(s.damage){}
    int id;//编号
    int health;//生命值
    int damage;//攻击力
    Kind kind;//种类
};
////////////////////////////////////////////////////////////////////////


///////////////设计子类:dragon 、ninja、iceman、lion、wolf///////////////
class Dragon:public Solider{
public:
    Dragon(int _id,int _health,int _damage):Solider(_id,_health,_damage){kind=dragon;}
    Dragon(const Dragon& d):Solider(d){kind=d.kind;}
};

class Ninja:public Solider{
public:
    Ninja(int _id,int _health,int _damage):Solider(_id,_health,_damage){kind=ninja;}
    Ninja(const Ninja& d):Solider(d){kind=d.kind;}
};

class Iceman:public Solider{
public:
    Iceman(int _id,int _health,int _damage):Solider(_id,_health,_damage){kind=iceman;}
    Iceman(const Iceman& d):Solider(d){kind=d.kind;}
};

class Lion:public Solider{
public:
    Lion(int _id,int _health,int _damage):Solider(_id,_health,_damage){kind=lion;}
    Lion(const Lion& d):Solider(d){kind=d.kind;}
};

class Wolf:public Solider{
public:
    Wolf(int _id,int _health,int _damage):Solider(_id,_health,_damage){kind=wolf;}
    Wolf(const Wolf& d):Solider(d){kind=d.kind;}
};
/////////////////////////////////////////////////////////////////////////


////////////////////////////设计司令部///////////////////////////////////
class Headquarters{
    //司令部类
public:
    Headquarters(Color col,int l):color(col),life_element(l),producing(true),number_of_soliders(0){
        memset(countmap,0,sizeof(countmap));
        if(life_element<*min_element(healthmap,healthmap+5))producing=false;
    }
    ~Headquarters(){
        for_each(soliders.begin(),soliders.end(),deleter());//清空司令部
    }
    //公有接口
    int get_number(){return number_of_soliders;}
    int get_life(){return life_element;}
    bool isproducing(){return producing;}
    void create(Kind k);
    void next();

private:
    Color color;//保存阵营
    vector<Solider*>soliders;//保存基类指针
    int number_of_soliders;//武士总数
    int life_element;//生命元
    int countmap[5];
    bool producing;//是否正在产生武士

};
/////////////////////////////////////////////////////////////////////////


/////////////////////////////成员函数////////////////////////////////////
void Headquarters::create(Kind k){
    switch(k){
        case dragon:{
            soliders.push_back(new Dragon(number_of_soliders+1,healthmap[k],damagemap[k]));break;
        }
        case ninja:{
            soliders.push_back(new Ninja(number_of_soliders+1,healthmap[k],damagemap[k]));break;
        }
        case iceman:{
            soliders.push_back(new Iceman(number_of_soliders+1,healthmap[k],damagemap[k]));break;
        }
        case lion:{
            soliders.push_back(new Lion(number_of_soliders+1,healthmap[k],damagemap[k]));break;
        }
        case wolf:{
            soliders.push_back(new Wolf(number_of_soliders+1,healthmap[k],damagemap[k]));break;
        }
    }
    number_of_soliders++;
    countmap[k]++;
    life_element-=healthmap[k];
    cout<<colorstr[color]<<" "<<kindstr[k]<<" "<<soliders[soliders.size()-1]->get_id()<<" "<<"born with strength "<<healthmap[k]<<",";
    cout<<countmap[k]<<" "<<kindstr[k]<<" in "<<colorstr[color]<<" headquarter"<<endl;
}



void Headquarters::next(){
    Kind lastkind;
    if(soliders.size()>0)lastkind=soliders[soliders.size()-1]->get_kind();//获取上一个武士的种类
    else lastkind=create_chain[color][4];//考虑创造第一个武士的特殊情形
    for(int i=1;i<=5;i++){
        Kind nextkind=create_chain[color][(op_create_chain[color][lastkind]+i)%5];
        if(healthmap[nextkind]<=life_element){
            create(nextkind);
            break;
        }
    }
    if(life_element<*min_element(healthmap,healthmap+5))producing=false;
}




//////////////////////////////功能函数/////////////////////////////////

void initialize(){
    cin>>LIFE_ELEMENT;//初始化生命元
    for(int i=0;i<5;i++){
        cin>>healthmap[i];//初始化生命值
    }
    redstop=bluestop=false;
}


void start(){
    //初始化司令部
    Headquarters RED(red,LIFE_ELEMENT);
    Headquarters BLUE(blue,LIFE_ELEMENT);
    for(int hour=0;;hour++){
        if(RED.isproducing()){
            cout<<setfill('0')<<setw(3)<<hour<<" ";
            RED.next();
        }
        else {
            if(!redstop){
                cout<<setfill('0')<<setw(3)<<hour<<" ";
                cout<<"red headquarter stops making warriors"<<endl;
                redstop=true;
            }
        }
        if(BLUE.isproducing()){
            cout<<setfill('0')<<setw(3)<<hour<<" ";
            BLUE.next();
        }
        else {
            if(!bluestop){
                cout<<setfill('0')<<setw(3)<<hour<<" ";
                cout<<"blue headquarter stops making warriors"<<endl;
                bluestop=true;
            }
        }
        if(redstop&&bluestop)break;
    }
}

///////////////////////////////////////////////////////////////////////////


////////////////////////////////主函数//////////////////////////////////////
int main(){
    int cases;
    cin>>cases;
    for(int i=1;i<=cases;i++){
        cout<<"Case:"<<i<<endl;
        initialize();
        start();
    }
    return 0;
}