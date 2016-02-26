#include "GameObject.h"
#include "GameStates.h"
#include <stdlib.h>
#include <time.h>

void GameObject::move()
{
	_pos = _pos + _speed;
}

BlockObject::BlockObject(const Vec2& pos, const Vec2& speed,
	float _half_width, float _half_height) :
	GameObject(pos, speed),
	half_width(_half_width),
	half_height(_half_height)
{
	Vec2 top;

	top.x = half_width;
	top.y = half_height;
	_shape.push_back(top);

	top.x = half_width;
	top.y = -half_height;
	_shape.push_back(top);

	top.x = -half_width;
	top.y = -half_height;
	_shape.push_back(top);

	top.x = -half_width;
	top.y = half_height;
	_shape.push_back(top);

	_hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);
}

bool BlockObject::intersect(std::vector<IntersectInfo>& intersections, GameObject &obj)
{
	Vec2 axis_x(1, 0);
	Vec2 axis_y(0, 1);
	float proect1_beg;
	float proect1_end;
	float inters_end;
	float inters_beg;
	IntersectInfo inters;

	obj.get_proection(axis_x, proect1_beg, proect1_end);
	inters_end = min(_pos.x + half_width, proect1_end);
	inters_beg = max(_pos.x - half_width, proect1_beg);
	if (inters_end - inters_beg < 0)
		return false;
	inters.normal = axis_x;
	inters.module = inters_end - inters_beg;
	intersections.push_back(inters);

	obj.get_proection(axis_y, proect1_beg, proect1_end);
	inters_end = min(_pos.y + half_height, proect1_end);
	inters_beg = max(_pos.y - half_height, proect1_beg);
	if (inters_end - inters_beg < 0)
		return false;
	inters.normal = axis_y;
	inters.module = inters_end - inters_beg;
	intersections.push_back(inters);

	return true;
}

void BlockObject::get_proection(const Vec2& axis, float& fst_coord, float& sec_coord)
{
	float proect;

	sec_coord = -1000000;
	fst_coord = 1000000;
	for (auto i = _shape.begin(); i != _shape.end(); i++)
	{
		proect = axis * ((*i)+_pos);
		fst_coord = min(fst_coord, proect);
		sec_coord = max(sec_coord, proect);
	}
}

void BlockObject::collide(const Vec2& normal)
{
	if (normal * _speed < 0)
		_speed = _speed.reflect(normal);
}

void BlockObject::draw(HDC hdc)
{
	Rectangle(hdc, _pos.x - half_width, _pos.y - half_height,
		_pos.x + half_width, _pos.y + half_height);
}


RoundObject::RoundObject(const Vec2& pos, const Vec2& speed,
	float radius) :	GameObject(pos, speed), _radius(radius)
{
	_shape.push_back(Vec2(0,0));
	_hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);
}

bool RoundObject::intersect(std::vector<IntersectInfo>& intersections, GameObject &obj)
{
	Vec2 axis;
	float min_range = 100000000;
	float proect_beg;
	float proect_end;
	float inters_beg;
	float inters_end;
	float center_proect;

	auto k = obj.shape().end();
	auto nearest_top = obj.shape().begin();
	for (auto i = obj.shape().begin(); i != k; i++)
	{
		float scalar = (_pos - ((*i) + obj.pos())) * (_pos - ((*i) + obj.pos()));
		if (scalar * scalar < min_range)
		{
			nearest_top = i;
			min_range = scalar * scalar;
		}
	}

	axis = ((*nearest_top) + obj.pos() - _pos).normalize();
	obj.get_proection(axis, proect_beg, proect_end);
	center_proect = _pos * axis;
	inters_end = min(center_proect + _radius, proect_end);
	inters_beg = max(center_proect - _radius, proect_beg);

	if (inters_end < inters_beg)
		return false;
	
	IntersectInfo intersection;
	intersection.module = inters_end - inters_beg;
	intersection.normal = axis;
	intersections.push_back(intersection);

	return true;
}

void RoundObject::get_proection(const Vec2& axis, float& fst_coord, float& sec_coord)
{
	float proect;

	proect = _pos * axis;
	sec_coord = proect + _radius;
	fst_coord = proect - _radius;
}

void RoundObject::collide(const Vec2& normal)
{
	if (normal * _speed < 0)
		_speed = _speed.reflect(normal);
}

void RoundObject::draw(HDC hdc)
{
	Ellipse(hdc, _pos.x - _radius, _pos.y - _radius,
		_pos.x + _radius, _pos.y + _radius);
}

void ArcanoidBlock::collide(const Vec2& normal)
{
	_health--;
	if (_health <= 0)
	{
		_owner->destroy(this);
		ArcanoidBlockFragment *new_frag;
		new_frag = new ArcanoidBlockFragment(Vec2(-1, -1), pos(), _owner);
		_owner->add_object(new_frag);
		new_frag = new ArcanoidBlockFragment(Vec2(-1.41, 0), pos(), _owner);
		_owner->add_object(new_frag);
		new_frag = new ArcanoidBlockFragment(Vec2(-1, 1), pos(), _owner);
		_owner->add_object(new_frag);
		new_frag = new ArcanoidBlockFragment(Vec2(0, 1.41), pos(), _owner);
		_owner->add_object(new_frag);
		new_frag = new ArcanoidBlockFragment(Vec2(1, 1), pos(), _owner);
		_owner->add_object(new_frag);
		new_frag = new ArcanoidBlockFragment(Vec2(1.41, 0), pos(), _owner);
		_owner->add_object(new_frag);
		new_frag = new ArcanoidBlockFragment(Vec2(1, -1), pos(), _owner);
		_owner->add_object(new_frag);
		new_frag = new ArcanoidBlockFragment(Vec2(0, -1.41), pos(), _owner);
		_owner->add_object(new_frag);

		_owner->owner()->_i_area._score.add(5);
	}
}

ArcanoidBlock::~ArcanoidBlock(){}

void ArcanoidBlockFragment::get_proection(const Vec2& axis, 
	float& fst_coord, float& sec_coord)
{
	fst_coord = 0;
	sec_coord = -1;
}

void ArcanoidBlockFragment::move()
{
	float scalar = (pos() - _block_pos) * (pos() - _block_pos);
	if (scalar * scalar > 200000)
		_owner->destroy(this);

	GameObject::move();
}

void Ball::move()
{
	if (_pos.y > GAME_AREA_Y - 5 && !_owner->is_game_over())
		_owner->on_game_over(false);
	GameObject::move();
}

void Ball::collide(const Vec2& normal)
{
	if (_pos.y <= GAME_AREA_Y - 5)
		RoundObject::collide(normal);
}

Board::Board(Ball *ball, GameArea *owner) 
	: RoundObject(Vec2(GAME_AREA_X / 2, GAME_AREA_Y + 100), Vec2(0, 0), 110), 
	_owner(owner), _ball(ball), has_a_ball(true)
{
	ball->_pos = Vec2(_pos.x, _pos.y - _radius - ball->_radius);
	ball->_speed = Vec2(0, 0);
}

void Board::move_to_left() {
	_speed = Vec2(-7, 0);
}

void Board::move_to_right() {
	_speed = Vec2(7, 0);
}

void Board::stop() {
	_speed = Vec2(0, 0);
}

void Board::move()
{
	_pos = _pos + _speed;
	_pos.x = max(50, _pos.x );
	_pos.x = min(GAME_AREA_X - 50, _pos.x);
	if (has_a_ball)
		_ball->_pos.x = _pos.x;
}

void Board::push_the_ball()
{
	if (!has_a_ball)
		return;

	srand(time(0));
	Vec2 start_speed;
	start_speed.y = -(rand() % 4 + 12);
	start_speed.x = rand() % 16 - 8;
	_ball->_speed = start_speed.normalize() * 5;
	_owner->owner()->_i_area._timer.run();

	has_a_ball = false;
}

void Board::collide(const Vec2& normal)
{
}