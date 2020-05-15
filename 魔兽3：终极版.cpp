#include<bits/stdc++.h>
//基于MinGW的预编译头文件
using namespace std;


/*Solider keep on marching on...*/


//构造deleter用于回收内存
struct deleter {
	template<typename T>
	void operator()(const T* ptr) const { delete ptr; }
};

////////////////////////////////全局变量////////////////////////////////
//声明
class City;
class Armament;
class Solider;
class Headquarters;
//

///////////游戏基本信息///////////
enum Color { red = 0, blue = 1 };//颜色阵营枚举
enum Kind { dragon = 0, ninja = 1, iceman = 2, lion = 3, wolf = 4 };//武士种类枚举
enum Arm { sword = 0, bomb = 1, arrow = 2 };//武器种类枚举
enum State { none = 0, win = 1, draw = 2, loss = 3, run = 4, killed_by_arrow = 5, kill_with_arrow = 6, killed_by_bomb = 7, easy_win = 8/*easy_win指当前城市的敌人被友军弓箭射死*/, escaping = 9 };//回合内战斗状态枚举
enum Flag { nothing = 0, redflag = 1, blueflag = 2 };//旗帜状态

pair<int, int>TIME;//记录全局时间（小时，分钟）
int minute[10] = { 0,5,10,20,30,35,38,40,50,55 };//关键时间点
vector<Armament*>weapon_bin;//回收无效武器

Kind create_chain[2][5] = { {iceman,lion,wolf,ninja,dragon},{lion,dragon,ninja,iceman,wolf} };//制造顺序
int op_create_chain[2][5] = { {4,3,0,1,2},{1,2,3,0,4} };//顺序逆映射
/////////////////////////////////


///////用于输出的字符映射////////
string colorstr[2] = { "red","blue" };
string kindstr[5] = { "dragon","ninja","iceman","lion","wolf" };
string armstr[3] = { "sword","bomb","arrow" };
////////////////////////////////


///////////初始化变量/////////////
vector<City*>Map;//全局地图

int healthmap[5] = { 0 };//生命值初始化
int damagemap[5] = { 0 };//攻击力初始化
int LIFE_ELEMENT = 0;//生命元
int N_cities = 0;//城市个数
int Arrow_damage = 0;//弓箭伤害
int dLoyalty = 0;//忠诚度减少量
int Final_time = 0;//结束运行的时间（分钟）
bool GAMEOVER = false;//游戏结束标志
////////////////////////////////



////////////////////////////////////////////////////////////////////////



///////////////////////////////设计武器//////////////////////////////////
class Armament {
public:
	Arm get_kind()const { return kind; }
	virtual void used() = 0;//使用武器时调用
	//////对五种武士开放信息许可/////
	friend class Solider;
	friend class Dragon;
	friend class Ninja;
	friend class Iceman;
	friend class Lion;
	friend class Wolf;
	//////////////////////////////
protected:
	Armament(Solider* sol, int d = 0) :holder(sol), damage(d), available(true), usage(0) {}
	Armament(Armament& a) {}
	Arm kind;
	int damage;
	int usage;//使用次数
	bool available;//武器是否可用
	Solider* holder;//武器持有者指针
};

class Sword :public Armament {
public:
	Sword(Solider* sol, int d = 0) :Armament(sol, d) {
		kind = sword;
		if (damage == 0)available = false;
	}
	Sword(Sword& s) :Armament(s) { kind = s.kind; }
	void used();
};
class Bomb :public Armament {
public:
	Bomb(Solider* sol) :Armament(sol) { kind = bomb; }
	Bomb(Bomb& s) :Armament(s) { kind = s.kind; }
	void used();
};
class Arrow :public Armament {
public:
	Arrow(Solider* sol, int d = 0) :Armament(sol, d) { kind = arrow; }
	Arrow(Arrow& s) :Armament(s) { kind = s.kind; }
	void used();
};

////////////////////////////////////////////////////////////////////////





///////////////////////////设计城市与司令部////////////////////////////////

class City {
public:
	City(int _id, int l = 0) :ID(_id), life_element(l), number_of_soliders(0), flag(nothing), last_two_wins(nothing, nothing), temp_life_elem(0) {}
	int get_ID()const { return ID; }
	int get_life()const { return life_element; }
	void generate_elem() { life_element += 10; }//产生生命元
	void set_attack_order();//设置攻击次序
	void change_wins(int c);//更改胜利情况
	void set_flags();//设置旗帜状况
	void reset();//重置城市
	vector<Solider*> get_solider() { return soliders; }

	//////对五种武士开放信息许可//////
	friend class Solider;
	friend class Dragon;
	friend class Ninja;
	friend class Iceman;
	friend class Lion;
	friend class Wolf;
	//////////////////////////////
protected:
	int ID;//城市编号
	Flag flag;//城市中的旗帜
	int life_element;//生命元
	int temp_life_elem;//生命元暂存
	int number_of_soliders;//武士总数
	vector<Solider*>soliders;//保存基类指针，存储当前城市的武士实时情况
	pair<Flag, Flag>last_two_wins;//记录前两次胜利情况
};



class Headquarters :public City {
	//司令部类
public:
	Headquarters(Color col, int l, int _id) :City(_id, l), color(col), producing(true), number_of_soliders_ever(0), conquered(false) {
		memset(countmap, 0, sizeof(countmap));
		if (life_element < create_chain[color][0])producing = false;
	}
	~Headquarters() {
		for_each(soliders_ever_created.begin(), soliders_ever_created.end(), deleter());//清除所有武士
	}
	//公有接口
	int get_number()const { return number_of_soliders; }
	Color get_color()const { return color; }
	bool isproducing()const { return producing; }
	bool isconquered()const { return conquered; }
	void create(Kind k);//产生特定种类的武士
	void create_next();//产生下一个武士
	void march_next();//驱动武士进军
	void check_state();//查看是否被占领
	void report_life();//报告生命元情况
	void award();//奖励武士

	//////对五种武士开放信息许可/////
	friend class Solider;
	friend class Dragon;
	friend class Ninja;
	friend class Iceman;
	friend class Lion;
	friend class Wolf;
	//////////////////////////////

private:
	Color color;//保存阵营
	int countmap[5];//保存各种武士的数量
	bool producing;//是否正常运转
	int number_of_soliders_ever;//该司令部创造的所有武士数量
	bool conquered;//司令部是否被占领
	vector<Solider*>soliders_ever_created;//该司令部创造的所有武士
};
/////////////////////////////////////////////////////////////////////////


////////////////////////////////设计武士/////////////////////////////////
class Solider {
	//武士类
public:
	//公有接口
	int get_id()const { return id; }
	int get_health()const { return health; }
	int get_damage()const { return damage; }
	Kind get_kind()const { return kind; }
	bool isfirsthand() { return first_hand; }
	bool Action() { return action; }
	Color get_color() { return color; }
	State get_state() { return state; }

	//功能函数
	virtual void born() = 0;//纯虚函数，武士出生时多态调用
	void death();//武士死亡
	void armed(Arm kind);//武士装备特定种类武器
	void catch_award();//获得司令部奖励
	void catch_elem();//取得城市中的生命元
	void attacked(int d) { health -= d; };//受击
	virtual void march_on();//前进(利用多态处理iceman的特殊情况)
	virtual void attack(Solider* object);//攻击(利用多态处理dragon的攻击)
	virtual void fight_back(Solider* object);//反击(利用多态处理ninja的反击)
	void reset();//回合结束，状态重置
	void earn_life();//获取城市中的生命元
	void shoot();//射箭
	bool explode();//释放炸弹
	void report_weapon();//报告武器信息
	virtual void escape() {}
	virtual void yell(bool easy = false) {}
	virtual void catch_weapon(Solider* object) {}//缴获武器

    //开放信息许可
	friend class City;
	friend class Headquarters;
	friend class Dragon;
	friend class Ninja;
	friend class Iceman;
	friend class Lion;
	friend class Wolf;
	friend class Sword;
	friend class Bomb;
	friend class Arrow;
	~Solider() {
		for_each(arms.begin(), arms.end(), deleter());//清空武器库
	}
protected:
	//受保护的构造函数，无法直接调用，防止基类对象实例化
	Solider(int _id, int _health, int _damage, Headquarters* h) :id(_id), health(_health), damage(_damage), camp(h), wins(0), state(none), first_hand(false), color(h->color), action(false) { position = camp; }
	Solider(const Solider& s) :id(s.id), health(s.health), damage(s.damage) {}
	int id;//编号
	int wins;//胜利次数
	Color color;
	Kind kind;//种类
	State state;//本回合战斗状态（战后统计）
	int health;//生命值
	int damage;//攻击力
	bool first_hand;//是否先手攻击
	bool action;//是否已经进军
	vector<Armament*>arms;//武器库
	Headquarters* camp;//表示所属司令部
	City* position;//表示所在位置
};
////////////////////////////////////////////////////////////////////////




///////////////设计子类:dragon 、ninja、iceman、lion、wolf///////////////
class Dragon :public Solider {
public:
	Dragon(int _id, int _health, int _damage, Headquarters* h) :Solider(_id, _health, _damage, h) { kind = dragon; }
	Dragon(const Dragon& d) :Solider(d) { kind = d.kind; }
	void born();
	void yell(bool easy = false);//欢呼
	void attack(Solider* object);
private:
	double morale;//士气
};

class Ninja :public Solider {
public:
	Ninja(int _id, int _health, int _damage, Headquarters* h) :Solider(_id, _health, _damage, h) { kind = ninja; }
	Ninja(const Ninja& d) :Solider(d) { kind = d.kind; }
	void born();
	void fight_back(Solider* object) {}//ninja受击时不反击
};

class Iceman :public Solider {
public:
	Iceman(int _id, int _health, int _damage, Headquarters* h) :Solider(_id, _health, _damage, h), steps(0) { kind = iceman; }
	Iceman(const Iceman& d) :Solider(d) { kind = d.kind; }
	void born();
	void march_on();
private:
	int steps;//行走步数
};

class Lion :public Solider {
public:
	Lion(int _id, int _health, int _damage, Headquarters* h) :Solider(_id, _health, _damage, h) { kind = lion; }
	Lion(const Lion& d) :Solider(d) { kind = d.kind; }
	void born();
	void escape();//lion逃跑
	void attack(Solider* object);
	void fight_back(Solider* object);
private:
	int loyalty;//忠诚度
};

class Wolf :public Solider {
public:
	Wolf(int _id, int _health, int _damage, Headquarters* h) :Solider(_id, _health, _damage, h) { kind = wolf; }
	Wolf(const Wolf& d) :Solider(d) { kind = d.kind; }
	void born();
	void attack(Solider* object);
	void fight_back(Solider* object);
	void catch_weapon(Solider* object);//缴获武器
};
/////////////////////////////////////////////////////////////////////////

/////////////////////////////成员函数////////////////////////////////////

void Solider::armed(Arm kind) {
	switch (kind) {
	case sword: {
		if (damage / 5 != 0)arms.push_back(new Sword(this, damage / 5)); break;
	}
	case bomb: {
		arms.push_back(new Bomb(this)); break;
	}
	case arrow: {
		arms.push_back(new Arrow(this, Arrow_damage)); break;
	}
	}
}



void Solider::death() {
	if (state != killed_by_arrow && state != killed_by_bomb && state != escaping) {
		cout << setfill('0') << setw(3) << TIME.first << ":";
		cout << setfill('0') << setw(2) << TIME.second << " ";
		cout << colorstr[color] << " " << kindstr[kind] << " " << id << " was killed in city " << position->ID << endl;
	}
	for (auto it = position->soliders.begin(); it != position->soliders.end(); it++) {
		if ((*it) == this) {
			position->soliders.erase(it);
			position->number_of_soliders--;
			break;//解除绑定
		}
	}
	position = NULL;
}



void Solider::catch_award() {
	if (camp->life_element >= 8) {
		if (state == win || state == easy_win) {
			health += 8;
			camp->life_element -= 8;
		}
	}
}

void Solider::reset() {
	state = none;
	first_hand = false;
	action = false;
}

void Solider::march_on() {
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

void Solider::attack(Solider* object) {
	int temp_health = object->health;
	int real_damage = damage;
	for (auto arm : arms) {
		if (arm->kind == sword && arm->available) { real_damage += arm->damage; arm->used(); }
	}
    object->attacked(real_damage);
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color] << " " << kindstr[kind] << " " << id << " attacked " << colorstr[object->color] << " " << kindstr[object->kind] << " " << object->id;
	cout << " in city " << position->ID << " with " << health << " elements and force " << damage << endl;
	if (object->health <= 0) {//攻击方获胜
		state = win;
		object->state = loss;
		position->change_wins(color);
		if (object->kind == lion)health += temp_health;//生命值转移
		object->death();
		wins++;
		earn_life();
	}
	else {
		object->fight_back(this);
		if (object->kind == ninja)position->change_wins(3);//考虑ninja不反击的平局情况
	}

}
void Solider::fight_back(Solider* object) {
	int temp_health = object->health;
	int real_damage = damage / 2;
	for (auto arm : arms) {
		if (arm->kind == sword && arm->available) { real_damage += arm->damage; arm->used(); }
	}
	object->attacked(real_damage);
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color] << " " << kindstr[kind] << " " << id << " fought back against " << colorstr[object->color] << " " << kindstr[object->kind] << " " << object->id;
	cout << " in city " << position->ID << endl;
	if (object->health <= 0) {//反击方获胜
		state = win;
		object->state = loss;
		position->change_wins(color);
		if (object->kind == lion)health += temp_health;//生命值转移
		object->death();
		wins++;
		earn_life();
	}
	else {//平局
		state = draw;
		object->state = draw;
		position->change_wins(3);
	}
}


void Solider::earn_life() {
	if (position->soliders.size() == 1 && position->ID != 0 && position->ID != N_cities + 1) {
		camp->temp_life_elem += position->life_element;//武士为所属司令部赢得生命元
		if (position->life_element > 0) {
			cout << setfill('0') << setw(3) << TIME.first << ":";
			cout << setfill('0') << setw(2) << TIME.second << " ";
			cout << colorstr[color] << " " << kindstr[kind] << " " << id << " earned " << position->life_element << " elements for his headquarter" << endl;
		}
		position->life_element = 0;
	}
}

void Solider::shoot() {
	City* nextposition=position;
	if(color==red) nextposition = Map[position->ID + 1];
	else if(color==blue)nextposition = Map[position->ID - 1];
	for (auto arm : arms) {
		if (arm->kind == arrow && arm->available) {
			for (auto s : nextposition->soliders) {
				if (s->color != color) {
					s->attacked(Arrow_damage);
					arm->used();
					cout << setfill('0') << setw(3) << TIME.first << ":";
					cout << setfill('0') << setw(2) << TIME.second << " ";
					cout << colorstr[color] << " " << kindstr[kind] << " " << id << " shot";
					if (s->health <= 0) {//攻击方获胜
						//state=kill_with_arrow;
						s->state = killed_by_arrow;
						cout << " and killed " << colorstr[s->color] << " " << kindstr[s->kind] << " " << s->id;
						for (auto s1 : nextposition->soliders) {
							if (s1->color == color) {
								if (s1->state != killed_by_arrow){s1->state = easy_win;}
								break;
							}//友军进入easy_win状态
						}
					}
					cout << endl;
					break;
				}
			}
			break;
		}
	}
}

bool Solider::explode() {
	for (auto Arm : arms) {
		if (Arm->kind == bomb) {
			int real_damage = damage;
			for (auto arm : arms) {
				if (arm->kind == sword && arm->available)real_damage += arm->damage;
			}
			int possible_harm = 0;//可能受到的伤害
			for (auto s : position->soliders) {
				if (s->color != color) {
					if (first_hand && real_damage < s->health && s->kind != ninja) {
						possible_harm += s->damage / 2;
						for (auto arm : s->arms) {
							if (arm->kind == sword && arm->available) {
								possible_harm += arm->damage;
							}
						}
					}
					else if (s->first_hand) {
						possible_harm += s->damage;
						for (auto arm : s->arms) {
							if (arm->kind == sword && arm->available) {
								possible_harm += arm->damage;
							}
						}
					}
					if (possible_harm >= health) {//如果有被杀死的风险，触发同归于尽机制
						cout << setfill('0') << setw(3) << TIME.first << ":";
						cout << setfill('0') << setw(2) << TIME.second << " ";
						cout << colorstr[color] << " " << kindstr[kind] << " " << id << " used a bomb and killed " << colorstr[s->color] << " " << kindstr[s->kind] << " " << s->id << endl;
						Arm->used();
						state = killed_by_bomb;
						s->state = killed_by_bomb;
						death();
						s->death();
						return true;
					}
					break;
				}
			}
		}
	}
	return false;
}


void Solider::report_weapon() {
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color] << " " << kindstr[kind] << " " << id << " has ";
	if (arms.empty())cout << "no weapon";
	else {
		int reported = 0;//声明的武器数量
		for (auto arm : arms) {
			if (arm->kind == arrow) {
				reported++;
				cout << "arrow(" << 3 - arm->usage << ")";
				if (arms.size() > reported)cout << ',';
			}
		}
		for (auto arm : arms) {
			if (arm->kind == bomb) {
				reported++;
				cout << "bomb";
				if (arms.size() > reported)cout << ',';
			}
		}
		for (auto arm : arms) {
			if (arm->kind == sword) {
				cout << "sword(" << arm->damage << ")";
			}
		}
	}
	cout << endl;
}


void Dragon::yell(bool easy) {
	if (easy)morale += 0.2;
	if (morale > 0.8 && first_hand == true) {
		cout << setfill('0') << setw(3) << TIME.first << ":";
		cout << setfill('0') << setw(2) << TIME.second << " ";
		cout << colorstr[color] << " dragon " << id << " yelled in city " << position->ID << endl;
	}
}
void Lion::escape() {
	if (loyalty <= 0) {
		state = run;
		cout << setfill('0') << setw(3) << TIME.first << ":";
		cout << setfill('0') << setw(2) << TIME.second << " ";
		cout << colorstr[color] << " lion " << id << " ran away" << endl;
		state = escaping;
		death();
	}
}
void Wolf::catch_weapon(Solider* object) {
	for (auto oarm : object->arms) {
		bool want = true;//是否拾取
		for (auto arm : arms) {
			if (arm->kind == oarm->kind)want = false;
		}
		if (want) {
			arms.push_back(oarm);
			oarm->holder = this;//更换主人
			for (auto iter = object->arms.begin(); iter != object->arms.end(); iter++) {
				if ((*iter) == oarm) { object->arms.erase(iter); break; }
			}
		}
	}
}


//////////////////////////////////多态////////////////////////////////////

void Sword::used() {
	damage = damage * 4 / 5;//磨损
	if (damage == 0) {
		available = false;
		for (auto iter = holder->arms.begin(); iter != holder->arms.end(); iter++) {
			if ((*iter) == this) { holder->arms.erase(iter); break; }
		}
		holder = NULL;
		weapon_bin.push_back(this);
	}
}
void Arrow::used() {
	usage++;
	if (usage == 3) {
		available = false;//使用次数耗尽
		for (auto iter = holder->arms.begin(); iter != holder->arms.end(); iter++) {
			if ((*iter) == this) { holder->arms.erase(iter); break; }
		}
		holder = NULL;
		weapon_bin.push_back(this);
	}
}
void Bomb::used() {
	available = false;//炸弹爆炸
	for (auto iter = holder->arms.begin(); iter != holder->arms.end(); iter++) {
		if ((*iter) == this) { holder->arms.erase(iter); break; }
	}
	holder = NULL;
	weapon_bin.push_back(this);
}



void Dragon::born() {
	armed((Arm)(id % 3));
	morale = (double)camp->life_element / (double)healthmap[kind];//计算士气
	cout << "Its morale is " << fixed << setprecision(2) << morale << endl;
}
void Ninja::born() {
	armed((Arm)(id % 3));
	armed((Arm)((id + 1) % 3));
}
void Iceman::born() {
	armed((Arm)(id % 3));
}
void Lion::born() {
	loyalty = camp->life_element;
	cout << "Its loyalty is " << loyalty << endl;
}
void Wolf::born() {}


void Iceman::march_on() {//Iceman前进时特殊处理
	position->number_of_soliders--;
	for (auto it = position->soliders.begin(); it != position->soliders.end(); it++) {
		if ((*it) == this) { position->soliders.erase(it); break; }//解除绑定
	}
	if (color == red) position = Map[position->ID + 1];//进入下一个城市
	else if(color==blue)position = Map[position->ID - 1];//进入下一个城市
	position->soliders.push_back(this);//重新绑定
	position->number_of_soliders++;
	action = true;
	steps++;
	if (steps != 0 && steps % 2 == 0) {
		if (health > 9)health -= 9;
		else health = 1;
		damage += 20;
	}
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

void Dragon::attack(Solider* object) {//Dragon攻击时特殊处理
	int temp_health = object->health;
	int real_damage = damage;
	for (auto arm : arms) {
		if (arm->kind == sword && arm->available) { real_damage += arm->damage; arm->used(); }
	}
	object->attacked(real_damage);
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color] << " " << kindstr[kind] << " " << id << " attacked " << colorstr[object->color] << " " << kindstr[object->kind] << " " << object->id;
	cout << " in city " << position->ID << " with " << health << " elements and force " << damage << endl;
	if (object->health <= 0) {//攻击方获胜
		state = win;
		object->state = loss;
		position->change_wins(color);
		if (object->kind == lion)health += temp_health;//生命值转移
		object->death();
		wins++;
		morale += 0.2;
	}
	else {
		object->fight_back(this);
		if (object->kind == ninja)position->change_wins(3);//考虑ninja不反击的平局情况
		morale -= 0.2;
	}
	if (state != loss)yell();//欢呼
	if (object->state == loss)earn_life();

}
void Lion::attack(Solider* object) {
	int temp_health = object->health;
	int real_damage = damage;
	for (auto arm : arms) {
		if (arm->kind == sword && arm->available) { real_damage += arm->damage; arm->used(); }
	}
	object->attacked(real_damage);
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color] << " " << kindstr[kind] << " " << id << " attacked " << colorstr[object->color] << " " << kindstr[object->kind] << " " << object->id;
	cout << " in city " << position->ID << " with " << health << " elements and force " << damage << endl;
	if (object->health <= 0) {//攻击方获胜
		state = win;
		object->state = loss;
		position->change_wins(color);
		if (object->kind == lion)health += temp_health;//生命值转移
		object->death();
		wins++;
		earn_life();
	}
	else {
		loyalty -= dLoyalty;
		object->fight_back(this);
		if (object->kind == ninja)position->change_wins(3);//考虑ninja不反击的平局情况
	}
}

void Lion::fight_back(Solider* object) {
	int temp_health = object->health;
	int real_damage = damage / 2;
	for (auto arm : arms) {
		if (arm->kind == sword && arm->available) { real_damage += arm->damage; arm->used(); }
	}
	object->attacked(real_damage);
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color] << " " << kindstr[kind] << " " << id << " fought back against " << colorstr[object->color] << " " << kindstr[object->kind] << " " << object->id;
	cout << " in city " << position->ID << endl;
	if (object->health <= 0) {//反击方获胜
		state = win;
		object->state = loss;
		position->change_wins(color);
		if (object->kind == lion)health += temp_health;//生命值转移
		object->death();
		wins++;
		earn_life();
	}
	else {//平局
		loyalty -= dLoyalty;
		state = draw;
		object->state = draw;
		position->change_wins(3);
	}
}
void Wolf::attack(Solider* object) {
	int temp_health = object->health;
	int real_damage = damage;
	for (auto arm : arms) {
		if (arm->kind == sword && arm->available) { real_damage += arm->damage; arm->used(); }
	}
	object->attacked(real_damage);
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color] << " " << kindstr[kind] << " " << id << " attacked " << colorstr[object->color] << " " << kindstr[object->kind] << " " << object->id;
	cout << " in city " << position->ID << " with " << health << " elements and force " << damage << endl;
	if (object->health <= 0) {//攻击方获胜
		state = win;
		object->state = loss;
		position->change_wins(color);
		if (object->kind == lion)health += temp_health;//生命值转移
		catch_weapon(object);//缴获武器
		object->death();
		wins++;
		earn_life();
	}
	else {
		object->fight_back(this);
		if (object->kind == ninja)position->change_wins(3);//考虑ninja不反击的平局情况
	}
}
void Wolf::fight_back(Solider* object) {
	int temp_health = object->health;
	int real_damage = damage / 2;
	for (auto arm : arms) {
		if (arm->kind == sword && arm->available) { real_damage += arm->damage; arm->used(); }
	}
	object->attacked(real_damage);
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color] << " " << kindstr[kind] << " " << id << " fought back against " << colorstr[object->color] << " " << kindstr[object->kind] << " " << object->id;
	cout << " in city " << position->ID << endl;
	if (object->health <= 0) {//反击方获胜
		state = win;
		object->state = loss;
		position->change_wins(color);
		if (object->kind == lion)health += temp_health;//生命值转移
		catch_weapon(object);//缴获武器
		object->death();
		wins++;
		earn_life();
	}
	else {//平局
		state = draw;
		object->state = draw;
		position->change_wins(3);
	}
}





/////////////////////////////////////////////////////////////////////////


void City::set_attack_order() {
	for (auto s : soliders) {
		if (s->color == red) {
			if (((ID % 2) && (flag == nothing)) || (flag == redflag))s->first_hand = true;
		}
		else if (s->color == blue) {
			if ((!(ID % 2) && (flag == nothing)) || (flag == blueflag))s->first_hand = true;
		}
	}
}


void City::set_flags() {//更换旗帜
	Flag last1 = last_two_wins.first;
	Flag last2 = last_two_wins.second;
	if (last1 == redflag && last2 == redflag) {
		if (flag == nothing || flag == blueflag) {
			flag = redflag;
			cout << setfill('0') << setw(3) << TIME.first << ":";
			cout << setfill('0') << setw(2) << TIME.second << " ";
			cout << "red flag raised in city " << ID << endl;
		}
		else if (flag == blueflag)flag = nothing;
	}
	else if (last1 == blueflag && last2 == blueflag) {
		if (flag == nothing || flag == redflag) {
			flag = blueflag;
			cout << setfill('0') << setw(3) << TIME.first << ":";
			cout << setfill('0') << setw(2) << TIME.second << " ";
			cout << "blue flag raised in city " << ID << endl;
		}
		else if (flag == redflag)flag = nothing;
	}
}


void City::change_wins(int c) {
	last_two_wins.first = last_two_wins.second;
	if (c == red)last_two_wins.second = redflag;
	else if (c == blue)last_two_wins.second = blueflag;
	else last_two_wins.second = nothing;
}
void City::reset(){
	for(auto s:soliders)s->reset();
}

void Headquarters::create(Kind k) {
	switch (k) {
	case dragon: {
		Solider* s = new Dragon(number_of_soliders_ever + 1, healthmap[k], damagemap[k], this);
		soliders.push_back(s); soliders_ever_created.push_back(s); break;
	}
	case ninja: {
		Solider* s = new Ninja(number_of_soliders_ever + 1, healthmap[k], damagemap[k], this);
		soliders.push_back(s); soliders_ever_created.push_back(s); break;
	}
	case iceman: {
		Solider* s = new Iceman(number_of_soliders_ever + 1, healthmap[k], damagemap[k], this);
		soliders.push_back(s); soliders_ever_created.push_back(s); break;
	}
	case lion: {
		Solider* s = new Lion(number_of_soliders_ever + 1, healthmap[k], damagemap[k], this);
		soliders.push_back(s); soliders_ever_created.push_back(s); break;
	}
	case wolf: {
		Solider* s = new Wolf(number_of_soliders_ever + 1, healthmap[k], damagemap[k], this);
		soliders.push_back(s); soliders_ever_created.push_back(s); break;
	}
	}
	number_of_soliders++;//增加武士数量
	number_of_soliders_ever++;
	countmap[k]++;//统计各种类武士数
	life_element -= healthmap[k];//减少生命元
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << colorstr[color] << " " << kindstr[k] << " " << number_of_soliders_ever << " " << "born" << endl;
	soliders_ever_created[number_of_soliders_ever - 1]->born();//通过多态实现不同种类武士的出生
}
void Headquarters::create_next() {
	Kind lastkind;
	if (soliders_ever_created.size() > 0)lastkind = soliders_ever_created[number_of_soliders_ever - 1]->get_kind();//获取上一个武士的种类
	else lastkind = create_chain[color][4];//考虑创造第一个武士的特殊情形
	Kind create_nextkind = create_chain[color][(op_create_chain[color][lastkind] + 1) % 5];//下一个武士种类
	if (healthmap[create_nextkind] <= life_element) { create(create_nextkind); producing = true; }
	else producing = false;
}

void Headquarters::march_next() {
	for (auto s : soliders_ever_created)s->march_on();
}

void Headquarters::check_state() {
	int counting = 0;
	for (auto s : soliders) {
		if (s->color != color)counting++;
	}
	if (counting >= 2) {
		conquered = true;
		cout << setfill('0') << setw(3) << TIME.first << ":";
		cout << setfill('0') << setw(2) << TIME.second << " ";
		cout << colorstr[color] << " headquarter was taken" << endl;
	}
}

void Headquarters::report_life() {
	cout << setfill('0') << setw(3) << TIME.first << ":";
	cout << setfill('0') << setw(2) << TIME.second << " ";
	cout << life_element << " elements in " << colorstr[color] << " headquarter" << endl;
}

void Headquarters::award() {
	//奖励武士，离敌方司令部近的优先
	if (color == red) {
		for (int i = N_cities; i > 0; i--) {
			for (auto s : Map[i]->get_solider()) {
				if (s->color == red)s->catch_award();
			}
		}
	}
	else if (color == blue) {
		for (int i = 1; i <= N_cities; i++) {
			for (auto s : Map[i]->get_solider()) {
				if (s->color == blue)s->catch_award();
			}
		}
	}
	life_element += temp_life_elem;
	temp_life_elem = 0;//获得之前武士得到的城市生命元
}

///////////////////////////////////////////////////////////////////////



//////////////////////////////功能函数/////////////////////////////////


void Escape() {//lion逃跑
	for (City* city : Map) {
		for (auto s : city->get_solider()) {
			if (s->get_kind() == lion)s->escape();
		}
	}
}


void March_on(Headquarters* RED, Headquarters* BLUE) {
	for (auto s : Map[1]->get_solider())if (s->get_color() == blue && !s->Action()) { s->march_on(); RED->check_state(); }
	for (int i = 0; i < N_cities; i++) {
		for (auto s : Map[i]->get_solider())if (s->get_color() == red && !s->Action())s->march_on();
		for (auto s : Map[i + 2]->get_solider())if (s->get_color() == blue && !s->Action())s->march_on();
	}
	for (auto s : Map[N_cities]->get_solider())if (s->get_color() == red && !s->Action()) { s->march_on(); BLUE->check_state(); }
}
void Generate() {//产生生命元
	for (City* city : Map) {
		if (city->get_ID() != 0 && city->get_ID() != N_cities + 1)city->generate_elem();
	}
}
void Earn_elem() {
	for (City* city : Map) {
		if (city->get_ID() != 0 && city->get_ID() != N_cities + 1) {
			for (auto s : city->get_solider()) {
				s->earn_life();
			}
		}
	}
}
void Shoot() {
	//射击
	for (City* city : Map) {
		if (city->get_ID() != 0 && city->get_ID() != N_cities + 1) {
			for (auto s : city->get_solider()) {
				s->shoot();
			}
		}
	}
	//wolf缴获武器
	for (City* city : Map) {
		if (city->get_ID() != 0 && city->get_ID() != N_cities + 1) {
			for (auto s : city->get_solider()) {
				if (s->get_kind() == wolf) {
					for (auto s1 : city->get_solider()) {
						if (s1->get_state() == killed_by_arrow) { s->catch_weapon(s1); break; }//wolf第一次拾取武器
					}
					break;
				}
			}
		}
	}
	//清理战场
	for (City* city : Map) {
		if (city->get_ID() != 0 && city->get_ID() != N_cities + 1) {
			for (auto s : city->get_solider()) {
				if (s->get_state() == killed_by_arrow)s->death();
                else if(s->get_state()==easy_win)city->change_wins(s->get_color());
			}
		}
	}
}

void Explode() {
	for (City* city : Map) {
		if (city->get_ID() != 0 && city->get_ID() != N_cities + 1) {
			city->set_attack_order();//设置攻击顺序
			for (auto s : city->get_solider()) {
				if (s->explode())break;
			}
		}
	}

}



void Attack() {
	for (City* city : Map) {
		if (city->get_ID() != 0 && city->get_ID() != N_cities + 1) {
			for (auto s : city->get_solider()) {
				if (s->get_state() == easy_win) { s->yell(true); s->earn_life(); }//easy_win武士获得城市生命元}
				else if (s->isfirsthand()) {
					for (auto s1 : city->get_solider()) {
						if (s1->get_color() != s->get_color() && !s1->isfirsthand())s->attack(s1);
					}
				}
			}
			city->set_flags();
		}
	}
}

void Report_weapon() {
	for (City* city : Map) {
		for (auto s : city->get_solider()) {
			if (s->get_color() == red)s->report_weapon();
		}
	}
	for (City* city : Map) {
		for (auto s : city->get_solider()) {
			if (s->get_color() == blue)s->report_weapon();
		}
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
	weapon_bin.clear();//清空武器库
	Map.clear();//清空地图
	GAMEOVER = false;
}

void initialize() {
	//初始化函数
	cin >> LIFE_ELEMENT >> N_cities >> Arrow_damage >> dLoyalty >> Final_time;//初始化
	for (int i = 0; i < 5; i++)cin >> healthmap[i];//初始化生命值
	for (int i = 0; i < 5; i++)cin >> damagemap[i];//初始化攻击力
}


void start(int T) {
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
		for (int tic = 0; tic < 10; tic++) {
			int min = minute[tic];
			if (hour * 60 + min > T)GAMEOVER = true;
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
				if (RED->isconquered() || BLUE->isconquered())GAMEOVER = true;
			}
			else if (min == 20)Generate();
			else if (min == 30)Earn_elem();
			else if (min == 35)Shoot();
			else if (min == 38)Explode();
			else if (min == 40) {
				Attack();
				RED->award();
				BLUE->award();
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
		start(Final_time);
	}
	return 0;
}
