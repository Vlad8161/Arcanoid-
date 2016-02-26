#include "GameStates.h"

//#define TRACE_FRAMES_FLAG

GameArea::GameArea(GSPlay *owner) : 
_owner(owner)
{
	GameObject *new_obj;
	_ball = new Ball(Vec2(0, 0), this);
	_board = new Board(_ball, this);

	for (int i = 0; i < 19; i++)
	{
		new_obj = new ArcanoidBlock(Vec2(40 + i * 40, 100), this);
		_objects.insert(new_obj);
		new_obj = new ArcanoidBlock(Vec2(40 + i * 40, 120), this);
		_objects.insert(new_obj);
		new_obj = new ArcanoidBlock(Vec2(40 + i * 40, 140), this);
		_objects.insert(new_obj);
	}

//	new_obj = new ArcanoidBlock(Vec2(GAME_AREA_X / 2, GAME_AREA_Y / 2), this);
//	_objects.insert(new_obj);

	new_obj = new BlockObject(Vec2(0, GAME_AREA_Y / 2), Vec2(0, 0),
		10, GAME_AREA_Y / 2 + 10);
	_objects.insert(new_obj);
	new_obj = new BlockObject(Vec2(GAME_AREA_X, GAME_AREA_Y / 2), Vec2(0, 0),
		10, GAME_AREA_Y / 2 + 10);
	_objects.insert(new_obj);
	new_obj = new BlockObject(Vec2(GAME_AREA_X / 2, 0), Vec2(0, 0),
		GAME_AREA_X / 2, 10);
	_objects.insert(new_obj);

	_game_over = false;
	_background = (HBRUSH)CreateSolidBrush(RGB(70, 120, 180));
}

GameArea::~GameArea()
{
	for (auto i = _objects.begin(); i != _objects.end(); i++)
		delete (*i);
	_objects.clear();

	delete _ball;
	delete _board;

	DeleteObject(_background);
}

void GameArea::on_game_over(bool success)
{
	_game_over = true;
	_owner->on_game_over(success);
}

void GameArea::calculate_next_frame()
{
	while (!_destroyed_objects.empty())
	{
		GameObject *current = _destroyed_objects.front();
		_destroyed_objects.pop_front();
		_objects.erase(current);
		delete current;
	}

	if (_objects.size() <= 3 && !_game_over)
	{
		_owner->on_game_over(true);
		_game_over = true;
	}

	for (auto i = _objects.begin(); i != _objects.end(); i++)
		(*i)->move();
	_board->move();
	_ball->move();

	for (auto i = _objects.begin(); i != _objects.end(); i++)
		try_collide(*(*i), *_ball);

	if (try_collide(*_ball, *_board))
		_owner->_i_area._score.reset_multiplier();
}

bool GameArea::try_collide(GameObject& obj1, GameObject& obj2)
{
	std::vector<IntersectInfo> intersections;
	intersections.reserve(3);
	int min = 0;

	if (!obj1.intersect(intersections, obj2) ||
		!obj2.intersect(intersections, obj1))
		return false;

	for (unsigned int i = 0; i < intersections.size(); i++)
		if (intersections[i].module < intersections[min].module)
			min = i;

	Vec2 reflection_normal = intersections[min].normal;
	if ((obj2.pos() - obj1.pos()) * reflection_normal > 0)
	{
		obj2.collide(reflection_normal);
		obj1.collide(reflection_normal.negate());
	}
	else
	{
		obj1.collide(reflection_normal);
		obj2.collide(reflection_normal.negate());
	}

	return true;
}

void GameArea::draw(HDC hdc)
{
	RECT rc = { 0 };

	rc.right = GAME_AREA_X;
	rc.bottom = GAME_AREA_Y;
	FillRect(hdc, &rc, _background);
	SelectObject(hdc, (HPEN)GetStockObject(NULL_PEN));
	for (auto i = _objects.begin(); i != _objects.end(); i++)
		(*i)->draw(hdc);
	_ball->draw(hdc);
	_board->draw(hdc);
}

void GameArea::on_tick()
{
	calculate_next_frame();
}

void GameArea::destroy(GameObject* obj_to_destroy)
{
	_destroyed_objects.push_back(obj_to_destroy);
}

void GameArea::add_object(GameObject *obj)
{
	_objects.insert(obj);
}

void GameArea::on_keydown(unsigned int virt_key_code)
{
	if (virt_key_code == VK_SPACE)
		_board->push_the_ball();

	if (virt_key_code == VK_LEFT)
		_board->move_to_left();
	else if (virt_key_code == VK_RIGHT)
		_board->move_to_right();
}

void GameArea::on_keyup(unsigned int virt_key_code)
{
	if (virt_key_code == VK_LEFT ||
		virt_key_code == VK_RIGHT)
		_board->stop();
}

InfoArea::InfoArea()
{
	LOGFONT lf;

	ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 56;
	lf.lfWeight = FW_EXTRABOLD;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(lf.lfFaceName, _T("Bauhaus 93"));
	_font = CreateFontIndirect(&lf);
}

InfoArea::~InfoArea()
{
	DeleteObject(_font);
}

void InfoArea::draw(HDC hdc)
{
	RECT rc = { 0, 0, INFO_AREA_X, INFO_AREA_Y };
	HDC hmem_timer;
	HDC hmem_score;
	HBITMAP hbmp_timer, hbmp_old_timer;
	HBITMAP hbmp_score, hbmp_old_score;
	HFONT old_font;

	FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(70, 120, 180));
	old_font = (HFONT)SelectObject(hdc, _font);
	rc.top += 10;
	DrawText(hdc, _T("Lab7.Arcanoid"), -1, &rc, 
		DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	SelectObject(hdc, old_font);

	hmem_timer = CreateCompatibleDC(hdc);
	hbmp_timer = CreateCompatibleBitmap(hdc, TIMER_RECT_X, TIMER_RECT_Y);
	hbmp_old_timer = (HBITMAP)SelectObject(hmem_timer, hbmp_timer);
	_timer.draw(hmem_timer);
	BitBlt(hdc, INFO_AREA_X - TIMER_RECT_X - 10, 10, TIMER_RECT_X, TIMER_RECT_Y,
		hmem_timer, 0, 0, SRCCOPY);

	hmem_score = CreateCompatibleDC(hdc);
	hbmp_score = CreateCompatibleBitmap(hdc, SCORE_RECT_X, SCORE_RECT_Y);
	hbmp_old_score = (HBITMAP)SelectObject(hmem_score, hbmp_score);
	_score.draw(hmem_score);
	BitBlt(hdc, 10, 10, SCORE_RECT_X, SCORE_RECT_Y,
		hmem_score, 0, 0, SRCCOPY);

	SelectObject(hmem_timer, hbmp_old_timer);
	SelectObject(hmem_score, hbmp_old_score);
	DeleteObject(hbmp_timer);
	DeleteObject(hbmp_score);
	DeleteDC(hmem_timer);
	DeleteDC(hmem_score);
}

void InfoArea::on_tick()
{
	_timer.on_tick();
}

void InfoArea::add(int delta_score)
{
	_score.add(delta_score);
}

GameStateContainer::GameStateContainer() : _states_to_pop(0)
{

}

GameStateContainer::~GameStateContainer()
{
	clear();
}

void GameStateContainer::push_state(AbstractGameState *new_state)
{
	_states_stack.push_back(new_state);
}

void GameStateContainer::pop_state(int how_many)
{
	_states_to_pop = how_many;
}

void GameStateContainer::update_state()
{
	while (_states_to_pop > 0 && _states_stack.size() != 0)
	{
		delete _states_stack.back();
		_states_stack.pop_back();
		_states_to_pop--;
	}

	if (_states_stack.size() == 0)
		PostQuitMessage(0);
}

void GameStateContainer::clear()
{
	for (auto i = _states_stack.begin(); i != _states_stack.end(); i++)
		delete *i;
	_states_stack.clear();
}

size_t GameStateContainer::size() {
	return _states_stack.size();
}

AbstractGameState* GameStateContainer::current(){
	return _states_stack.back();
}

GSPlay::GSPlay(GameStateContainer *owner, AbstractGameState *prev = 0) : 
	AbstractGameState(owner, prev), _game_over(false), _g_area(this) 
{
}

LPARAM AbstractGameState::ConvertCoord(LPARAM phisical_coord, RECT& rc)
{
	int real_frame_height = rc.bottom;
	int real_frame_width = real_frame_height / 3 * 4;

	int x = (LOWORD(phisical_coord)) - (rc.right - real_frame_width) / 2;
	x = x * FRAME_X / real_frame_width;
	int y = HIWORD(phisical_coord) * FRAME_Y / real_frame_height;
	return LPARAM((y << 16) | (x & 0x0000FFFF));
}

void GSPlay::render(HDC hdc)
{
	HDC hmem_game_area;
	HDC hmem_info_area;
	HBITMAP hbmp_new_game, hbmp_new_info;
	HBITMAP hbmp_old_game, hbmp_old_info;

	hmem_game_area = CreateCompatibleDC(hdc);
	hbmp_new_game = CreateCompatibleBitmap(hdc, GAME_AREA_X, GAME_AREA_Y);
	hbmp_old_game = (HBITMAP)SelectObject(hmem_game_area, hbmp_new_game);
	_g_area.draw(hmem_game_area);

	hmem_info_area = CreateCompatibleDC(hdc);
	hbmp_new_info = CreateCompatibleBitmap(hdc, INFO_AREA_X, INFO_AREA_Y);
	hbmp_old_info = (HBITMAP)SelectObject(hmem_info_area, hbmp_new_info);
	_i_area.draw(hmem_info_area);

	BitBlt(hdc, 0, INFO_AREA_Y, GAME_AREA_X, GAME_AREA_Y, hmem_game_area, 0, 0, SRCCOPY);
	BitBlt(hdc, 0, 0, INFO_AREA_X, INFO_AREA_Y, hmem_info_area, 0, 0, SRCCOPY);

	SelectObject(hmem_game_area, hbmp_old_game);
	SelectObject(hmem_info_area, hbmp_old_info);
	DeleteObject(hbmp_new_game);
	DeleteObject(hbmp_new_info);
	DeleteDC(hmem_game_area);
	DeleteDC(hmem_info_area);
}

void GSPlay::on_game_over(bool success)
{
	_game_over = true;
	_i_area._timer.stop();

	GSMessage *msg;
	if (success)
		msg = new GSMessage(_owner, this, _T("YOU WON"));
	else
		msg = new GSMessage(_owner, this, _T("YOU LOSE"));
	_owner->push_state(msg);
}

void GSPlay::input(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
#ifdef TRACE_FRAMES_FLAG
		case WM_KEYDOWN:
#else
		case WM_TIMER:
#endif
			_i_area.on_tick();
			_g_area.on_tick();
			break;

#ifndef TRACE_FRAMES_FLAG
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
				_owner->pop_state(1);
			else
				_g_area.on_keydown((unsigned int)wParam);
			break;
#endif
			
		case WM_KEYUP:
			_g_area.on_keyup((unsigned int)wParam);
			break;
	}
}

GSMainMenu::GSMainMenu(GameStateContainer* owner) : 
AbstractGameState(owner), 
_btn_new_game(_T("New Game"), FRAME_X / 2 - 100, 440, 200, 46),
_btn_exit(_T("Exit"), FRAME_X / 2 - 100, 500, 200, 46)
{
	LOGFONT lf;

	ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 80;
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(lf.lfFaceName, _T("Bauhaus 93"));
	_font = CreateFontIndirect(&lf);

	_background = CreateSolidBrush(RGB(70, 120, 180));
}

GSMainMenu::~GSMainMenu()
{
	DeleteObject(_background);
}

void GSMainMenu::render(HDC hdc)
{
	RECT rc = { 0, 0, FRAME_X, FRAME_Y };
	HFONT old_font = (HFONT)SelectObject(hdc, _font);

	FillRect(hdc, &rc, _background);
	
	rc.bottom -= 400;
	SetTextColor(hdc, RGB(255, 255, 255));
	SetBkMode(hdc, TRANSPARENT);
	DrawText(hdc, _T("Arcanoid++"), -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	SelectObject(hdc, old_font);

	_btn_new_game.draw(hdc);
	_btn_exit.draw(hdc);
}

void GSMainMenu::input(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_LBUTTONUP:
		if (_btn_new_game.is_inside(LOWORD(lParam), HIWORD(lParam)))
		{
			GSPlay *new_game = new GSPlay(_owner, this);
			_owner->push_state(new_game);
		}
		else if (_btn_exit.is_inside(LOWORD(lParam), HIWORD(lParam)))
			_owner->pop_state(1);
	}
}

GSMessage::GSMessage(GameStateContainer *owner, AbstractGameState *prev, const TCHAR *text) :
AbstractGameState(owner, prev)
{
	size_t text_len = _tcslen(text);
	_text = new TCHAR[text_len + 1];
	_tcscpy_s(_text, text_len + 1, text);

	_border = CreatePen(PS_SOLID, 10, RGB(255, 255, 255));
	_background = CreateSolidBrush(RGB(70, 120, 180));

	LOGFONT lf;

	ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 60;
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(lf.lfFaceName, _T("Bauhaus 93"));
	_font = CreateFontIndirect(&lf);
}

GSMessage::~GSMessage()
{
	delete[]_text;
	DeleteObject(_font);
	DeleteObject(_background);
	DeleteObject(_border);
}

void GSMessage::render(HDC hdc)
{
	_prev_state->render(hdc);

	HPEN old_pen = (HPEN)SelectObject(hdc, _border);
	HBRUSH old_brush = (HBRUSH)SelectObject(hdc, _background);
	RECT rc1, rc2;

	rc1.left = (FRAME_X - message_width) / 2;
	rc1.top = (FRAME_Y - message_height) / 2;
	rc1.right = rc1.left + message_width;
	rc1.bottom = rc1.top + message_height;
	Rectangle(hdc, rc1.left, rc1.top, rc1.right, rc1.bottom);

	rc2 = rc1;
	rc1.bottom -= 100;
	rc2.top += 100;

	HFONT old_font = (HFONT)SelectObject(hdc, _font);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 255));
	DrawText(hdc, _text, -1, &rc1, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	SelectObject(hdc, old_font);
	
	DrawText(hdc, _T("Press SPACE to continue..."), -1, &rc2,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	SelectObject(hdc, old_pen);
	SelectObject(hdc, old_brush);
}

void GSMessage::input(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_TIMER:
		_prev_state->input(msg, wParam, lParam);
		break;
	case WM_KEYUP:
		if (wParam == VK_SPACE)
			_owner->pop_state(2);
		break;
	}
}

Timer::Timer()
{
	LOGFONT lf;

	ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 70;
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(lf.lfFaceName, _T("Bauhaus 93"));
	_font = CreateFontIndirect(&lf);
	own_font = true;

	stop();
	_last_second_tick = GetTickCount();
	_minutes = 0;
	_seconds = 0;
	_background = CreateSolidBrush(RGB(70, 120, 180));
}

Timer::Timer(HFONT font) :
_font(font)
{
	own_font = false;
	_last_second_tick = GetTickCount();
	_minutes = 0;
	_seconds = 0;
	_background = CreateSolidBrush(RGB(70, 120, 180));
}

Timer::~Timer()
{
	DeleteObject(_background);
	if (own_font)
		DeleteObject(_font);
}

void Timer::draw(HDC hdc)
{
	RECT rc = { 0, 0, TIMER_RECT_X, TIMER_RECT_Y };
	HFONT old_font = (HFONT)SelectObject(hdc, _font);
	char time_line[6];

	FillRect(hdc, &rc, _background);
	sprintf_s(time_line, "%02d:%02d", min(_minutes, 59), _seconds);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 255));
	DrawTextA(hdc, time_line, -1, &rc,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	SelectObject(hdc, old_font);
}

void Timer::on_tick()
{
	if (!_is_ticking)
		return;
	unsigned int new_tick_count;

	new_tick_count = GetTickCount();
	if (new_tick_count - _last_second_tick > 1000)
	{
		_last_second_tick = new_tick_count;
		if (++_seconds >= 60)
		{
			_minutes++;
			_seconds -= 60;
		}
	}
}

void Timer::stop()
{
	_is_ticking = false;
}

void Timer::run(){
	_is_ticking = true;
}

Score::Score(int score) :_score(score)
{
	LOGFONT lf;

	ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 70;
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(lf.lfFaceName, _T("Bauhaus 93"));
	_font = CreateFontIndirect(&lf);
	own_font = true;

	_multiplier = 1;
	_background = CreateSolidBrush(RGB(70, 120, 180));
}

Score::Score(HFONT font, int score) :
_font(font), _score(score)
{
	own_font = false;
	_background = CreateSolidBrush(RGB(70, 120, 180));
}

Score::~Score()
{
	DeleteObject(_background);
	if (own_font)
		DeleteObject(_font);
}

void Score::add(int val)
{
	_score = min(_score + (val * _multiplier), 9999);
	_score = max(_score, 0);
	_multiplier++;
}

void Score::reset()
{
	_score = 0;
	_multiplier = 1;
}

void Score::reset_multiplier()
{
	_multiplier = 1;
}

void Score::draw(HDC hdc)
{
	RECT rc = { 0, 0, SCORE_RECT_X, SCORE_RECT_Y };
	HFONT old_font = (HFONT)SelectObject(hdc, _font);
	char time_line[6];

	FillRect(hdc, &rc, _background);
	sprintf_s(time_line, "%04d", _score);

	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 255));
	DrawTextA(hdc, time_line, -1, &rc,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	SelectObject(hdc, old_font);
}

Button::Button(const TCHAR* text, int x, int y, int cx, int cy)
{
	unsigned int text_len = _tcslen(text);
	_text = new TCHAR[text_len + 1];
	_tcscpy_s(_text, text_len+1, text);
	_location.left = x;
	_location.top = y;
	_location.right = cx + x;
	_location.bottom = cy + y;

	HRGN round_rgn = CreateEllipticRgn(x, y, x + 10, y + 10);
	HRGN horz_rgn = CreateRectRgn(x, y + 5, _location.right, _location.bottom - 5);
	_rgn = CreateRectRgn(x + 5, y, _location.right - 5, _location.bottom);
	
	CombineRgn(_rgn, _rgn, horz_rgn, RGN_OR);
	CombineRgn(_rgn, _rgn, round_rgn, RGN_OR);
	OffsetRgn(round_rgn, cx - 10, 0);
	CombineRgn(_rgn, _rgn, round_rgn, RGN_OR);
	OffsetRgn(round_rgn, 0, cy - 10);
	CombineRgn(_rgn, _rgn, round_rgn, RGN_OR);
	OffsetRgn(round_rgn, 10 - cx, 0);
	CombineRgn(_rgn, _rgn, round_rgn, RGN_OR);
	DeleteObject(round_rgn);
	DeleteObject(horz_rgn);

	LOGFONT lf;
	ZeroMemory(&lf, sizeof(lf));
	lf.lfHeight = 40;
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = ANTIALIASED_QUALITY;

	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(lf.lfFaceName, _T("Bauhaus 93"));
	_font = CreateFontIndirect(&lf);

	_text_color = RGB(70, 120, 180);
	_background_color = (HBRUSH)GetStockObject(WHITE_BRUSH);
}

Button::~Button()
{
	delete[]_text;
	DeleteObject(_font);
	DeleteObject(_rgn);
}

void Button::draw(HDC hdc)
{
	HFONT old_font = (HFONT)SelectObject(hdc, _font);

	FillRgn(hdc, _rgn, _background_color);
	SetTextColor(hdc, _text_color);
	DrawText(hdc, _text, -1, &_location, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	SelectObject(hdc, old_font);
}

bool Button::is_inside(int x, int y)
{
	return PtInRegion(_rgn, x, y);
}