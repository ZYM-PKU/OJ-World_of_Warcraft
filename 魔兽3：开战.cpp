#include<bits/stdc++.h>
using namespace std;


/*Soldier keep on marching on...*/


//构造deleter用于回收内存
struct deleter {
	template<typename T>
	void operator()(const T* ptr) const { delete ptr; }
};

////////////////////////////////全局变量////////////////////////////////

//声明
class Armament;
class City;
class Headquarters;
class Soldier;


///////////游戏基本信息///////////
enum Color { red = 0, blue = 1 };//颜色阵营枚举
enum Kind { dragon = 0, ninja = 1, iceman = 2, lion = 3, wolf = 4 };//武士种类枚举
enum Arm { sword = 0, bomb = 1, arrow = 2 };//武器种类枚举

pair<int, int>TIME;//记录全局时间（小时，分钟）
int minute[7] = { 0,5,10,35,40,50,55 };//关键时间点
Kind create_chain[2][5] = { {iceman,lion,wolf,ninja,dragon},{lion,dragon,ninja,iceman,wolf} };//制造顺序


string colorstr[2] = { "red","blue" };
string kindstr[5] = { "dragon","ninja","iceman","lion","wolf" };
string armstr[3] = { "sword","bomb","arrow" };
////////////////////////////////



///////////初始化变量/////////////
vector<City*>Map;//全局地图
vector<Armament*>weapon_bin;//回收无效武器
vector<Soldier*>dead_soliders;//处理死亡武士

int healthmap[5] = { 0 };//生命值初始化
int damagemap[5] = { 0 };//攻击力初始化
int LIFE_ELEMENT = 0;//生命元
int N_cities = 0;//城市个数
int dLoyalty = 0;//忠诚度减少量
int Final_time = 0;//结束运行的时间（分钟）
bool GAMEOVER = false;//游戏结束标志
////////////////////////////////



////////////////////////////////////////////////////////////////////////



///////////////////////////////设计武器//////////////////////////////////
class Armament {
	//武器类
public:
	Arm get_kind() { return kind; }
	int get_damage() { return damage; }
	int get_usage() { return usage; }
	bool get_av() { return available; }
	Soldier* get_holder(){ return holder; }
	void set_holder(Soldier* s) { holder = s; }
	void broken();

	virtual void used(Soldier* object) = 0;//使用武器时调用
	virtual void reset_damage() = 0;//重置攻击力

protected:
	Armament(Soldier* sol, int d, Arm k) :holder(sol), damage(d), kind(k), available(true), usage(0) {}
	Armament(Armament& a) {}
	Arm kind;
	int damage;
	int usage;//使用次数
	bool available;//武器是否可用
	Soldier* holder;//武器持有者指针
};

class Sword :public Armament {
public:
	Sword(Soldier* sol, int d) :Armament(sol, d, sword) {}
	void used(Soldier* object);
	void reset_damage();
};
class Bomb :public Armament {
public:
	Bomb(Soldier* sol, int d) :Armament(sol, d, bomb) {}
	void used(Soldier* object);
	void reset_damage();
};
class Arrow :public Armament {
public:
	Arrow(Soldier* sol, int d) :Armament(sol, d, arrow) {}
	void used(Soldier* object);
	void reset_damage();
};

////////////////////////////////////////////////////////////////////////





///////////////////////////设计城市与司令部////////////////////////////////

class City {
	//城市类
public:
	City(int _id) :ID(_id), number_of_soliders(0) {}

	int get_ID()const { return ID; }
	int get_number()const { return number_of_soliders; }
	vector<Soldier*> get_solider() { return soliders; }
	void set_attack_order();//设置攻击次序
	void set_weapon_order();//设置武器顺序
	void reset();//重置城市

	/////对武士开放信息许可/////
	friend class Soldier;

protected:
	int ID;//城市编号
	int number_of_soliders;//武士总数
	vector<Soldier*>soliders;//保存基类指针，存储当前城市的武士实时情况
};



class Headquarters :public City {
	//司令部类
public:
	Headquarters(Color col, int l, int _id) :City(_id), life_element(l), color(col), producing(true),conquered(false),
	number_of_soliders_ever(0),curr_index(0) {
		if (life_element < create_chain[color][0])producing = false;
	}

	//公有接口
	Color get_color() { return color; }
	int get_life_element() { return life_element; }
	bool is_producing() { return producing; }
	bool is_conquered() { return conquered; }
	void create(Kind k);//产生特定种类的武士
	void create_next();//产生下一个武士
	void check_state();//查看是否被占领
	void report_life();//报告生命元情况


private:
	Color color;//保存阵营
	int life_element;//生命元
	bool producing;//是否正常运转
	bool conquered;//司令部是否被占领

	int number_of_soliders_ever;//已生成武士数量
	int curr_index;//当前生成武士种类下标
};
/////////////////////////////////////////////////////////////////////////


////////////////////////////////设计武士/////////////////////////////////
class Soldier {
	//武士类
public:
	//公有接口
	int get_id() { return id; }
	Color get_color() { return color; }
	Kind get_kind() { return kind; }
	bool is_dead() { return dead; }
	int get_health() { return health; }
	int get_damage() { return damage; }
	bool is_first_hand() { return first_hand; }
	bool Action() { return action; }
	vector<Armament*> get_arms() { return arms; }

	//功能函数
	void death();//武士死亡
	void armed(Arm kind);//武士装备特定种类武器
	void hurt(int d) { health -= d; };//受击
	void reset();//回合结束，状态重置
	void sort_weapon();//排序武器
	void report_weapon();//报告武器信息
	void drop_weapon(Armament* a);//丢掉武器
	void set_first_hand(bool f){ first_hand = f; }
	void catch_weapon(Soldier* object);//缴获武器

	virtual void born();//纯虚函数，武士出生时多态调用
	virtual void escape() {}//Lion逃跑
	virtual void yell() {}//Dragon欢呼
	virtual void catch_weapon_wolf(Soldier* object) {};//wolf缴获武器
	virtual void march_on();//前进(利用多态处理行军)
	virtual void attack(Soldier* object);//攻击(利用多态处理攻击)


	~Soldier() {
		for_each(arms.begin(), arms.end(), deleter());//清空武器库
	}

protected:
	//受保护的构造函数，无法直接调用，防止基类对象实例化
	Soldier(int _id, int _health, int _damage, Headquarters* h) :id(_id),
	health(_health), damage(_damage), camp(h), dead(0),
	first_hand(false), color(h->get_color()), action(false), curr_index(0) { position = camp; }

	Soldier(const Soldier& s) :id(s.id), health(s.health), damage(s.damage) {}

	int id;//编号
	Color color;//阵营
	Kind kind;//种类
	bool dead;//是否死亡
	int health;//生命值
	int damage;//攻击力
	bool first_hand;//是否先手攻击
	bool action;//是否已经进军
	vector<Armament*>arms;//武器库

	Headquarters* camp;//表示所属司令部
	City* position;//表示所在位置
	int curr_index;//当前使用武器下标
};



class Dragon :public Soldier {
public:
	Dragon(int _id, int _health, int _damage, Headquarters* h) :Soldier(_id, _health, _damage, h) { kind = dragon; }
	Dragon(const Dragon& d) :Soldier(d) { kind = d.kind; }
	void born();
	void yell();//欢呼
};

class Ninja :public Soldier {
public:
	Ninja(int _id, int _health, int _damage, Headquarters* h) :Soldier(_id, _health, _damage, h) { kind = ninja; }
	Ninja(const Ninja& d) :Soldier(d) { kind = d.kind; }
	void born();
};

class Iceman :public Soldier {
public:
	Iceman(int _id, int _health, int _damage, Headquarters* h) :Soldier(_id, _health, _damage, h) { kind = iceman; }
	Iceman(const Iceman& d) :Soldier(d) { kind = d.kind; }
	void born();
	void march_on();
};

class Lion :public Soldier {
public:
	Lion(int _id, int _health, int _damage, Headquarters* h) :Soldier(_id, _health, _damage, h) { kind = lion; }
	Lion(const Lion& d) :Soldier(d) { kind = d.kind; }
	void born();
	void march_on();
	void escape();//lion逃跑
private:
	int loyalty;//忠诚度
};

class Wolf :public Soldier {
public:
	Wolf(int _id, int _health, int _damage, Headquarters* h) :Soldier(_id, _health, _damage, h) { kind = wolf; }
	Wolf(const Wolf& d) :Soldier(d) { kind = d.kind; }
	void born();
	void catch_weapon_wolf(Soldier* object);//wolf缴获武器
};
/////////////////////////////////////////////////////////////////////////



/////////////////////////////成员函数////////////////////////////////////


void Armament::broken() {
	holder->drop_weapon(this);
	available = false;
	holder = NULL;
	weapon_bin.push_back(this);
}


void Sword::reset_damage() {
	damage = holder->get_damage() / 5;
}

void Bomb::reset_damage() {
	damage = holder->get_damage()*2 / 5;
}

void Arrow::reset_damage() {
	damage = holder->get_damage()*3 / 10;
}


void City::set_attack_order() {
	for (auto s : soliders) {
		if (s->get_color() == red) {
			if (ID % 2==1)s->set_first_hand(true);
			else s->set_first_hand(false);
		}
		else if (s->get_color() == blue) {
			if (ID % 2==0)s->set_first_hand(true);
			else s->set_first_hand(false);
		}
	}
}

void City::set_weapon_order() {
	for (auto s : soliders) s->sort_weapon();
}


void City::reset(){
	for(auto s:soliders) s->reset();
}



void Soldier::born() {
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color] << " " << kindstr[kind] << " " << id << " " << "born" << endl;
}

void Soldier::armed(Arm kind) {
	switch (kind) {
		case sword: {
			arms.push_back(new Sword(this, damage / 5)); break;
		}
		case bomb: {
			arms.push_back(new Bomb(this, damage*2 / 5)); break;
		}
		case arrow: {
			arms.push_back(new Arrow(this, damage*3 / 10)); break;
		}
		default: {}
	}
}


void Soldier::death() {

	for (auto it = position->soliders.begin(); it != position->soliders.end(); it++) {
		if ((*it) == this) {
			position->soliders.erase(it);//解除绑定
			position->number_of_soliders--;
			break;
		}
	}
	position = NULL;
	dead = true;
	dead_soliders.push_back(this);
	
}


void Soldier::reset() {
	first_hand = false;
	action = false;
}

void Soldier::march_on() {

	position->number_of_soliders--;
	for (auto it = position->soliders.begin(); it != position->soliders.end(); it++) {
		if ((*it) == this) { position->soliders.erase(it); break; }//解除绑定
	}

	if (color == red) position = Map[position->ID + 1];//进入下一个城市
	else if(color==blue)position = Map[position->ID - 1];//进入下一个城市

	position->soliders.push_back(this);//重新绑定
	position->number_of_soliders++;
	action = true;
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";

	if (position->ID != N_cities + 1&&position->ID != 0) {
		cout << colorstr[color] << " " << kindstr[kind] << " " << id << " " << "marched to city " << position->ID << " with ";
		cout << health << " elements and force " << damage << endl;
	}
	else {
		cout << colorstr[color] << " " << kindstr[kind] << " " << id << " reached " << colorstr[!color] << " headquarter with ";
		cout << health << " elements and force " << damage << endl;
	}

}

void Soldier::attack(Soldier* object) {

	if(arms.size()==0)return;

	Armament* curr_arm = arms[curr_index];
	curr_arm->used(object);

	if(arms.size()!=0){
		if(!curr_arm->get_av())
			curr_index %= arms.size();
		else
			curr_index = (curr_index + 1) % arms.size();//选择下一个武器
	}
	else curr_index = 0;
		

	if (object->health <= 0)
		object->death();
	if (health <= 0)
		death();

}

void Soldier::catch_weapon(Soldier* object) {

	auto oarms = object->arms;
	if(oarms.size()==0)return;
	sort(oarms.begin(),oarms.end(),[&](Armament* a1,Armament* a2){
		return a1->get_kind()==a2->get_kind()? a1->get_usage()<a2->get_usage():a1->get_kind()<a2->get_kind();
	});

	for (auto oarm : oarms) {
		if (arms.size()<=10) {
			arms.push_back(oarm);
			oarm->set_holder(this);//更换主人
			oarm->reset_damage();//更新攻击力
			for (auto iter = object->arms.begin(); iter != object->arms.end(); iter++) {
				if ((*iter) == oarm) { object->arms.erase(iter); break; }
			}
		}
		else break;
	}

}

void Soldier::sort_weapon() {
	curr_index = 0;
	sort(arms.begin(),arms.end(),[&](Armament* a1,Armament* a2){
		return a1->get_kind()==a2->get_kind()? a1->get_usage()>a2->get_usage():a1->get_kind()<a2->get_kind();
	});
}

void Soldier::report_weapon() {

	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color] << " " << kindstr[kind] << " " << id << " has ";
	
	int sword_count = 0, bomb_count = 0, arrow_count = 0;

	for (auto arm : arms) {
		if (arm->get_kind() == sword) {
			sword_count++;
		}
		else if(arm->get_kind() == bomb){
			bomb_count++;
		}
		else arrow_count++;
	}

	cout<<sword_count<<" sword "<<bomb_count<<" bomb "<<arrow_count<<" arrow";
	cout <<" and "<<health<<" elements"<< endl;

}


void Soldier::drop_weapon(Armament* a){
	for (auto iter = arms.begin(); iter != arms.end(); iter++) {
		if ((*iter) == a) { arms.erase(iter); break; }
	}
}


/////////////////////////////////////////////////////////////////////////



///////////////////////////////武器多态//////////////////////////////////

void Sword::used(Soldier* object) {
	object->hurt(damage);
}

void Arrow::used(Soldier* object) {
	object->hurt(damage);
	if (++usage >= 2) {
		available = false;
		holder->drop_weapon(this);
	}
}

void Bomb::used(Soldier* object) {
	object->hurt(damage);
	if(holder->get_kind() != ninja)holder->hurt(damage/2);
	available = false;
	holder->drop_weapon(this);
}

/////////////////////////////////////////////////////////////////////////



//////////////////////////////司令部多态/////////////////////////////////

void Headquarters::create(Kind k) {

	Soldier* s;
	switch (k) {
		case dragon: {
			s = new Dragon(number_of_soliders_ever + 1, healthmap[k], damagemap[k], this);
			soliders.push_back(s); break;
		}
		case ninja: {
			s = new Ninja(number_of_soliders_ever + 1, healthmap[k], damagemap[k], this);
			soliders.push_back(s); break;
		}
		case iceman: {
			s = new Iceman(number_of_soliders_ever + 1, healthmap[k], damagemap[k], this);
			soliders.push_back(s); break;
		}
		case lion: {
			s = new Lion(number_of_soliders_ever + 1, healthmap[k], damagemap[k], this);
			soliders.push_back(s); break;
		}
		case wolf: {
			s = new Wolf(number_of_soliders_ever + 1, healthmap[k], damagemap[k], this);
			soliders.push_back(s); break;
		}
		default: {}
	}

	number_of_soliders++;//增加武士数量
	number_of_soliders_ever++;
	life_element -= healthmap[k];//减少生命元
	s->born();//武士出生

}

void Headquarters::create_next() {

	if(!producing)return;
	Kind create_nextkind = create_chain[color][curr_index];//下一个武士种类
	curr_index = (curr_index + 1) % 5;
	
	if (healthmap[create_nextkind] <= life_element)create(create_nextkind); 
	else producing = false;

}


void Headquarters::check_state() {

	for (auto s : soliders) {
		if (s->get_color() != color){
			conquered = true;
			cout << setfill('0') << setw(3) << TIME.first << ":";
			cout << setfill('0') << setw(2) << TIME.second << " ";
			cout << colorstr[color] << " headquarter was taken" << endl;
			break;
		}
	}

}

void Headquarters::report_life() {

	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << life_element << " elements in " << colorstr[color] << " headquarter" << endl;

}

///////////////////////////////////////////////////////////////////////



///////////////////////////////武士多态//////////////////////////////////

void Dragon::born() {
	Soldier::born();
	armed((Arm)(id % 3));
}
void Ninja::born() {
	Soldier::born();
	armed((Arm)(id % 3));
	armed((Arm)((id + 1) % 3));
}
void Iceman::born() {
	Soldier::born();
	armed((Arm)(id % 3));
}
void Lion::born() {
	Soldier::born();
	armed((Arm)(id % 3));
	loyalty = camp->get_life_element();
	cout << "Its loyalty is " << loyalty << endl;
}
void Wolf::born() {
	Soldier::born();
}


void Iceman::march_on() {//Iceman前进时特殊处理
	health -= health/10;
	Soldier::march_on();
}

void Lion::march_on() {//Lion前进时特殊处理
	loyalty -= dLoyalty;
	Soldier::march_on();
}


void Dragon::yell() {

	if (!dead) {
		cout << setfill('0') << setw(3) << TIME.first << ":";
		cout << setfill('0') << setw(2) << TIME.second << " ";
		cout << colorstr[color] << " dragon " << id << " yelled in city " << position->get_ID() << endl;
	}

}

void Lion::escape() {

	if (loyalty <= 0) {
		cout << setfill('0') << setw(3) << TIME.first << ":";
		cout << setfill('0') << setw(2) << TIME.second << " ";
		cout << colorstr[color] << " lion " << id << " ran away" << endl;
		death();
	}

}

void Wolf::catch_weapon_wolf(Soldier* object) {//wolf缴获武器

	auto oarms = object->get_arms();
	if(oarms.size()==0)return;

	sort(oarms.begin(),oarms.end(),[&](Armament* a1,Armament* a2){
		return a1->get_kind()==a2->get_kind()? a1->get_usage()<a2->get_usage():a1->get_kind()<a2->get_kind();
	});

	Arm kind = oarms[0]->get_kind();
	int count = 0;
	for (auto oarm : oarms) {
		if (oarm->get_kind()==kind && arms.size()<10) {
			arms.push_back(oarm);
			oarm->set_holder(this);//更换主人
			oarm->reset_damage();//更新攻击力
			object->drop_weapon(oarm);
			count++;
		}
		else break;
	}

	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color]<<" wolf "<<id<<" took "<<count<<" "<<armstr[kind];
	cout << " from "<<colorstr[object->get_color()]<<" "<<kindstr[object->get_kind()]<<" "<<object->get_id();
	cout << " in city "<<position->get_ID()<<endl;

}
/////////////////////////////////////////////////////////////////////////




//////////////////////////////功能函数/////////////////////////////////


void Escape() {//lion逃跑

	for (City* city : Map) {
		for (auto s : city->get_solider()) {
			if (s->get_kind() == lion)s->escape();
		}
	}

}

void March_on(Headquarters* RED, Headquarters* BLUE) {

	for (auto s : Map[1]->get_solider())
		if (s->get_color() == blue && !s->Action()) { s->march_on(); RED->check_state(); }

	for (int i = 0; i < N_cities; i++) {
		for (auto s : Map[i]->get_solider())if (s->get_color() == red && !s->Action())s->march_on();
		for (auto s : Map[i + 2]->get_solider())if (s->get_color() == blue && !s->Action())s->march_on();
	}

	for (auto s : Map[N_cities]->get_solider())
		if (s->get_color() == red && !s->Action()) { s->march_on(); BLUE->check_state(); }

}


void Grab() {

	for (City* city : Map) {
		if (city->get_ID() != 0 && city->get_ID() != N_cities + 1) {
			for (auto s : city->get_solider()) {
				if (s->get_kind() == wolf){
					for (auto s1 : city->get_solider()) {
						if (s1->get_kind() != wolf) { s->catch_weapon_wolf(s1); break; }//wolf拾取武器
					}
					break;
				}
			}
		}
	}

}


void Attack() {

	for (City* city : Map) {
		if (city->get_ID() != 0 && city->get_ID() != N_cities + 1) {

			city->set_attack_order();//设置攻击顺序
			city->set_weapon_order();//设置武器顺序

			Soldier* s1 = NULL;
			Soldier* s2 = NULL;

			for (auto s : city->get_solider()) {
				if (s->is_first_hand()) s1=s;
				else s2=s;		
			}

			if(!s1||!s2)continue;

			cout << setfill('0') << setw(3) << TIME.first << ":";
			cout << setfill('0') << setw(2) << TIME.second << " ";

			while(1){
				s1->attack(s2);
				if(!s2->is_dead())s2->attack(s1);

				if(s1->is_dead()&&s2->is_dead()){
					Soldier* sr = s1->get_color()==red? s1:s2;
					Soldier* sb = s1->get_color()==blue? s1:s2;
					cout<<"both red "<< kindstr[sr->get_kind()]<<" "<<sr->get_id();
					cout<<" and blue "<< kindstr[sb->get_kind()]<<" "<<sb->get_id();
					cout<<" died in city "<<city->get_ID()<<endl;
					break;
				}
				else if(s1->is_dead()){
					cout<<colorstr[s2->get_color()]<<" "<< kindstr[s2->get_kind()]<<" "<<s2->get_id();
					cout<<" killed "<<colorstr[s1->get_color()]<<" "<< kindstr[s1->get_kind()]<<" "<<s1->get_id();
					cout<<" in city "<<city->get_ID();
					cout<<" remaining "<<s2->get_health()<<" elements"<<endl;
					s2->catch_weapon(s1);
					s2->yell();
					break;
				}
				else if(s2->is_dead()){
					cout<<colorstr[s1->get_color()]<<" "<< kindstr[s1->get_kind()]<<" "<<s1->get_id();
					cout<<" killed "<<colorstr[s2->get_color()]<<" "<< kindstr[s2->get_kind()]<<" "<<s2->get_id();
					cout<<" in city "<<city->get_ID();
					cout<<" remaining "<<s1->get_health()<<" elements"<<endl;
					s1->catch_weapon(s2);
					s1->yell();
					break;
				}
				else{
					bool draw = true;
					for(auto a:s1->get_arms()){
						if((a->get_kind()==sword&&a->get_damage()>0)||(a->get_kind()!=sword&&a->get_av()))draw=false;
					}
					for(auto a:s2->get_arms()){
						if((a->get_kind()==sword&&a->get_damage()>0)||(a->get_kind()!=sword&&a->get_av()))draw=false;
					}
					if(draw){
						Soldier* sr = s1->get_color()==red? s1:s2;
						Soldier* sb = s1->get_color()==blue? s1:s2;
						cout<<"both red "<< kindstr[sr->get_kind()]<<" "<<sr->get_id();
						cout<<" and blue "<< kindstr[sb->get_kind()]<<" "<<sb->get_id();
						cout<<" were alive in city "<<city->get_ID()<<endl;

						sr->yell();
						sb->yell();
						break;
					}
				}
			}

		}
	}

}

void Report_weapon() {

	for (City* city : Map) {
		Soldier* sr = NULL;
		Soldier* sb = NULL;
		
		for (auto s : city->get_solider()) {
			if (s->get_color() == red)sr = s;
			else sb = s;
		}

		if(sr)sr->report_weapon();
		if(sb)sb->report_weapon();
	}

}

void Reset_solider() {

	for (City* city : Map) {
		if (city->get_ID() != 0 && city->get_ID() != N_cities + 1) {
			for (auto s : city->get_solider()) {
				s->reset();
			}
		}
	}

}


void RESET(Headquarters* RED, Headquarters* BLUE) {

	delete RED;
	delete BLUE;
	for_each(Map.begin() + 1, Map.end() - 1, deleter());//清理堆内存
	for_each(weapon_bin.begin(), weapon_bin.end(), deleter());//清理堆内存
	for_each(dead_soliders.begin(), dead_soliders.end(), deleter());//清理堆内存
	weapon_bin.clear();//清空武器库
	dead_soliders.clear();//清空死亡武士
	Map.clear();//清空地图
	GAMEOVER = false;

}


void initialize() {
	//初始化函数
	cin >> LIFE_ELEMENT >> N_cities >> dLoyalty >> Final_time;//初始化
	for (int i = 0; i < 5; i++)cin >> healthmap[i];//初始化生命值
	for (int i = 0; i < 5; i++)cin >> damagemap[i];//初始化攻击力
}


void start() {
	///////////////////初始化地图///////////////////
	Headquarters* RED = new Headquarters(red, LIFE_ELEMENT, 0);
	Headquarters* BLUE = new Headquarters(blue, LIFE_ELEMENT, N_cities + 1);

	Map.push_back(RED);
	for (int i = 1; i <= N_cities; i++) {
		Map.push_back(new City(i));
	}
	Map.push_back(BLUE);

	/////////////运行时间线（核心进程）///////////////
	for (int hour = 0;; hour++) {
		for (int tic = 0; tic < 7; tic++) {
			int min = minute[tic];
			if (hour * 60 + min > Final_time)GAMEOVER = true;
            if (GAMEOVER)break;
			TIME.first = hour;
			TIME.second = min;//更新时间
			if (min == 0) {//司令部创造武士
				RED->create_next();
				BLUE->create_next();
			}
			else if (min == 5)Escape();
			else if (min == 10) {//武士进军
				March_on(RED, BLUE);
				if (RED->is_conquered() || BLUE->is_conquered())GAMEOVER = true;
			}
			else if (min == 35)Grab();
			else if (min == 40) {
				Attack();
				Reset_solider();//重置士兵状态
			}
			else if (min == 50) {
				RED->report_life();
				BLUE->report_life();
			}
			else if (min == 55)Report_weapon();
			if (GAMEOVER)break;
		}
		if (GAMEOVER)break;
	}
	RESET(RED, BLUE);//全局重置
}

///////////////////////////////////////////////////////////////////////////



////////////////////////////////主函数//////////////////////////////////////
int main() {
	int cases;
	cin >> cases;
	for (int i = 1; i <= cases; i++) {
		cout << "Case " << i << ":" << endl;
		initialize();
		start();
	}
	return 0;
}
