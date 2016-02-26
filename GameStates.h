#ifndef _GAME_STATES_H
#define _GAME_STATES_H

#include "Vec2.h"
#include "GameObject.h"
#include <windows.h>
#include <sstream>
#include <tchar.h>
#include <list>
#include <set>

class GameStateContainer;
class AbstractGameState;
class GSMainMenu;
class GSPlay;
class Score;
class Timer;
class Button;
class GameArea;
class InfoArea;

class Button {
	HRGN _rgn;
	RECT _location;
	TCHAR *_text;
	HFONT _font;
	COLORREF _text_color;
	HBRUSH _background_color;

public:
	Button(const TCHAR* text, int x, int y, int cx, int cy);
	~Button();

	void draw(HDC hdc);
	bool is_inside(int x, int y);
};

class Score
{
	int _score;
	unsigned int _multiplier;
	HBRUSH _background;
	HFONT _font;
	bool own_font;

public:
	Score(int score = 0);
	Score(HFONT font, int score = 0);
	virtual ~Score();
	void add(int val);
	void reset();
	void reset_multiplier();
	void draw(HDC hdc);
};

class Timer
{
	HFONT _font;
	HBRUSH _background;
	unsigned int _last_second_tick;
	unsigned int _minutes; 
	unsigned int _seconds;
	bool own_font;
	bool _is_ticking;

public:
	Timer();
	Timer(HFONT _font);
	virtual ~Timer();
	void stop();
	void run();
	void on_tick();
	void draw(HDC hdc);	
};

class GameArea
{
	std::set<GameObject*> _objects;
	std::list<GameObject*> _destroyed_objects;
	Ball *_ball;
	Board *_board;
	HBRUSH _background;
	GSPlay *_owner;
	bool _game_over;

	void calculate_next_frame();
	static bool try_collide(GameObject& obj1, GameObject& obj2);

public:
	GameArea(GSPlay *owner);
	virtual ~GameArea();

	void on_keydown(unsigned int virt_key_code);
	void on_keyup(unsigned int virt_key_code);
	void on_tick();
	void on_game_over(bool success);
	void draw(HDC hdc);
	void destroy(GameObject *obj_to_destroy);
	void add_object(GameObject *new_game_object);
	GSPlay *owner(){
		return _owner;
	}
	bool is_game_over() {
		return _game_over;
	}
};

class InfoArea
{
	HFONT _font;
public:
	Score _score;
	Timer _timer;
	
	InfoArea();
	virtual ~InfoArea();
	void on_tick();
	void add(int delta_score);
	void draw(HDC hdc);
};

class GameStateContainer
{
	std::list<AbstractGameState*> _states_stack;
	int _states_to_pop;

public:
	GameStateContainer();
	virtual ~GameStateContainer();

	void pop_state(int how_many);
	void push_state(AbstractGameState *new_state);
	void update_state();
	void clear();
	size_t size();
	AbstractGameState* current();
};

class AbstractGameState
{
protected:
	GameStateContainer* _owner;
	AbstractGameState* _prev_state;

public:
	AbstractGameState(GameStateContainer* owner, AbstractGameState* prev = 0) : 
		_owner(owner), _prev_state(prev) {}
	virtual ~AbstractGameState(){}
	virtual void render(HDC hdc) = 0;
	virtual void input(UINT msg, WPARAM wParam, LPARAM lParam) = 0;
	static LPARAM ConvertCoord(LPARAM phisical_coord, RECT& rc);
};

class GSMainMenu : public AbstractGameState
{
	HFONT _font;
	HBRUSH _background;
	Button _btn_new_game;
	Button _btn_exit;

public:
	GSMainMenu(GameStateContainer* owner);
	~GSMainMenu();
	virtual void render(HDC hdc);
	virtual void input(UINT msg, WPARAM wParam, LPARAM lParam);
};

class GSMessage : public AbstractGameState
{
	enum {
		message_width = 300,
		message_height = 250
	};

	HFONT _font;
	HBRUSH _background;
	HPEN _border;
	TCHAR  *_text;

public:
	GSMessage(GameStateContainer *owner, AbstractGameState *prev, const TCHAR *text);
	~GSMessage();
	virtual void render(HDC hdc);
	virtual void input(UINT msg, WPARAM wParam, LPARAM lParam);
};

class GSPlay : public AbstractGameState
{
	bool _game_over;

public:
	GameArea _g_area;
	InfoArea _i_area;

	GSPlay(GameStateContainer *owner, AbstractGameState *prev);
	void render(HDC hdc);
	void input(UINT msg, WPARAM wParam, LPARAM lParam);
	void on_game_over(bool success);
};



#endif
