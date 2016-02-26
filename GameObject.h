#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#define GAME_AREA_X		800 
#define GAME_AREA_Y		530
#define FRAME_X			800
#define FRAME_Y			600
#define INFO_AREA_X		800
#define INFO_AREA_Y		70
#define TIMER_RECT_X	200
#define TIMER_RECT_Y	(INFO_AREA_Y-10)
#define SCORE_RECT_X	200
#define SCORE_RECT_Y	(INFO_AREA_Y-10)

#include "Vec2.h"
#include <Windows.h>
#include <vector>

class GameArea;

typedef struct {
	Vec2 normal;
	float module;
} IntersectInfo;

class GameObject
{

protected:
	HBRUSH _hbr;
	Vec2 _pos;
	Vec2 _speed;
	std::vector<Vec2> _shape;

public:

	GameObject(const Vec2& pos = { 0, 0 }, const Vec2& speed = { 0, 0 }) :
		_pos(pos), _speed(speed) {}
	virtual ~GameObject(){
	}

	std::vector<Vec2>& shape(){
		return _shape;
	}

	Vec2 pos(){
		return _pos;
	}

	Vec2 speed(){
		return _speed;
	}

	virtual void collide(const Vec2& normal) = 0;
	virtual bool intersect(std::vector<IntersectInfo>& intersections, GameObject& obj) = 0;
	virtual void get_proection(const Vec2& axis, float& fst_coord, float& sec_coord) = 0;
	virtual void draw(HDC hdc) = 0;

	virtual void move();
};

class BlockObject : public GameObject
{
	const float half_width;
	const float half_height;

public:
	BlockObject(const Vec2& pos = { 0, 0 }, const Vec2& speed = { 0, 0 },
		float _half_width = 30, float _half_height = 30);
	~BlockObject(){}

	bool intersect(std::vector<IntersectInfo>& intersections, GameObject& obj);
	void get_proection(const Vec2& axis, float& fst_coord, float& sec_coord);
	void collide(const Vec2& normal);
	void draw(HDC hdc);
};

class RoundObject : public GameObject
{
protected:
	const float _radius;

public:
	RoundObject(const Vec2& pos = { 0, 0 }, const Vec2& speed = { 0, 0 },
		float radius = 5);
	~RoundObject(){}

	bool intersect(std::vector<IntersectInfo>& intersections, GameObject& obj);
	void get_proection(const Vec2& axis, float& fst_coord, float& sec_coord);
	void collide(const Vec2& normal);
	void draw(HDC hdc);
};

class ArcanoidBlock : public BlockObject
{
	GameArea *_owner;
	int _health;

public:
	ArcanoidBlock(const Vec2& pos, GameArea* owner) :
		BlockObject(pos, Vec2(0, 0), 20, 10),
		_owner(owner), _health(1) {}
	~ArcanoidBlock();

	void collide(const Vec2& normal);
};

class ArcanoidBlockFragment : public RoundObject
{
	GameArea *_owner;
	Vec2 _block_pos;

public:
	ArcanoidBlockFragment(const Vec2& pos,
		const Vec2& block_pos, GameArea *owner) :
		RoundObject(pos + block_pos, pos, 3), _owner(owner), _block_pos(block_pos) {}

	bool intersect(std::vector<IntersectInfo>& intersections, GameObject& obj){
		return false;
	}
	void get_proection(const Vec2& axis, float& fst_coord, float& sec_coord);
	void move();
};

class Board;

class Ball : public RoundObject
{
	friend Board;
	GameArea *_owner;

public:
	Ball(const Vec2& pos, GameArea *owner) :
		RoundObject(pos, Vec2(0, 0), 5), _owner(owner) {}

	void move();
	void collide(const Vec2& normal);
};

class Board : public RoundObject
{
	Ball *_ball;
	GameArea *_owner;
	bool has_a_ball;

public:
	Board(Ball *ball, GameArea *owner);
	void collide(const Vec2& normal);
	void move_to_left();
	void move_to_right();
	void stop();
	void push_the_ball();
	void move();
};

#endif