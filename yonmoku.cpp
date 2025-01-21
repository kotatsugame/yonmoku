#include<iostream>
#include<queue>
#include<vector>
#include<array>
#include<algorithm>
#include<cassert>
#include<random>
#include<set>
#include<map>
#include<chrono>

using namespace std;

mt19937 rng(random_device{}());
//mt19937 rng;
enum Cell{None, Me, You};
enum State{Continue, End, Invalid};
enum Color{Draw, Black, White};

const int SIZE = 4;
#define IDX(x, y, z) ((x) + (y) * SIZE + (z) * SIZE * SIZE)
#define BIT(x, y, z) (1uLL << IDX(x, y, z))
#define X(idx) ((idx) % SIZE)
#define Y(idx) ((idx) / SIZE % SIZE)
#define Z(idx) ((idx) / SIZE / SIZE)
const int BOARD_SIZE = SIZE * SIZE * SIZE;
const int INF = 1e9;
const int LINES_NUM = 76;

array<unsigned long long, LINES_NUM> LINES;
static const unsigned long long move_order[] = {
	BIT(0, 0, 0) | BIT(0, 3, 0) | BIT(3, 0, 0) | BIT(3, 3, 0) |
	BIT(1, 1, 1) | BIT(1, 2, 1) | BIT(2, 1, 1) | BIT(2, 2, 1) |
	BIT(1, 1, 2) | BIT(1, 2, 2) | BIT(2, 1, 2) | BIT(2, 2, 2) |
	BIT(0, 0, 3) | BIT(0, 3, 3) | BIT(3, 0, 3) | BIT(3, 3, 3),// first core + corner
	0xffffffffffff0000uLL,//second 2-4 layer
	0xffffffffffffffffuLL,// last
};

struct Board
{
	unsigned long long Me, You;
	Board() : Me(0uLL), You(0uLL) {}

	static inline enum State win(const unsigned long long B)
	{
		{//z dir
			unsigned long long b = B & B >> 2 * SIZE * SIZE;
			//static const unsigned long long mask = 0x000000000000ffffuLL;
			if (b & b >> SIZE * SIZE) return State::End;
		}
		{//x dir
			unsigned long long b = B & B >> 2;
			static const unsigned long long mask = 0x1111111111111111uLL;
			if (b & b >> 1 & mask) return State::End;
		}
		{//y dir
			unsigned long long b = B & B >> 2 * SIZE;
			static const unsigned long long mask = 0x000f000f000f000fuLL;
			if (b & b >> SIZE & mask) return State::End;
		}
		{//yz dir
			{
				unsigned long long b = B & B >> 2 * (SIZE * SIZE + SIZE);
				//static const unsigned long long mask = 0x000000000000000fuLL;
				if (b & b >> SIZE * SIZE + SIZE) return State::End;
			}
			{
				unsigned long long b = B & B >> 2 * (SIZE * SIZE - SIZE);
				static const unsigned long long mask = 0x000000000000f000uLL;
				if (b & b >> SIZE * SIZE - SIZE & mask) return State::End;
			}
		}
		{//zx dir
			{
				unsigned long long b = B & B >> 2 * (SIZE * SIZE + 1);
				static const unsigned long long mask = 0x0000000000001111uLL;
				if (b & b >> SIZE * SIZE + 1 & mask) return State::End;
			}
			{
				unsigned long long b = B & B >> 2 * (SIZE * SIZE - 1);
				static const unsigned long long mask = 0x0000000000008888uLL;
				if (b & b >> SIZE * SIZE - 1 & mask) return State::End;
			}
		}
		{//xy dir
			{
				unsigned long long b = B & B >> 2 * (SIZE + 1);
				static const unsigned long long mask = 0x0001000100010001uLL;
				if (b & b >> SIZE + 1 & mask) return State::End;
			}
			{
				unsigned long long b = B & B >> 2 * (SIZE - 1);
				static const unsigned long long mask = 0x0008000800080008uLL;
				if (b & b >> SIZE - 1 & mask) return State::End;
			}
		}
		{//xyz dir
			{
				static const unsigned long long line = BIT(0, 0, 0) | BIT(1, 1, 1) | BIT(2, 2, 2) | BIT(3, 3, 3);
				if ((B & line) == line) return State::End;
			}
			{
				static const unsigned long long line = BIT(0, 0, 3) | BIT(1, 1, 2) | BIT(2, 2, 1) | BIT(3, 3, 0);
				if ((B & line) == line) return State::End;
			}
			{
				static const unsigned long long line = BIT(0, 3, 0) | BIT(1, 2, 1) | BIT(2, 1, 2) | BIT(3, 0, 3);
				if ((B & line) == line) return State::End;
			}
			{
				static const unsigned long long line = BIT(0, 3, 3) | BIT(1, 2, 2) | BIT(2, 1, 1) | BIT(3, 0, 0);
				if ((B & line) == line) return State::End;
			}
		}
		return State::Continue;
	}

	static inline unsigned long long reach(const unsigned long long B)
	{
		unsigned long long ret = 0uLL;
		{//z dir
			const unsigned long long b = B & B >> SIZE * SIZE;
			ret |= B << SIZE * SIZE & b << 3 * SIZE * SIZE;
			ret |= B >> SIZE * SIZE & b << 2 * SIZE * SIZE;
			ret |= b >> SIZE * SIZE & B << SIZE * SIZE;
			ret |= b >> 2 * SIZE * SIZE & B >> SIZE * SIZE;
		}
		{//x dir
			const unsigned long long b = B & B >> 1;
			{
				static const unsigned long long mask = 0x1111111111111111uLL;
				ret |= mask & B >> 1 & b >> 2;
			}
			{
				static const unsigned long long mask = 0x2222222222222222uLL;
				ret |= mask & B << 1 & b >> 1;
			}
			{
				static const unsigned long long mask = 0x4444444444444444uLL;
				ret |= mask & b << 2 & B >> 1;
			}
			{
				static const unsigned long long mask = 0x8888888888888888uLL;
				ret |= mask & b << 3 & B << 1;
			}
		}
		{//y dir
			const unsigned long long b = B & B >> SIZE;
			{
				static const unsigned long long mask = 0x000f000f000f000fuLL;
				ret |= mask & B >> SIZE & b >> 2 * SIZE;
			}
			{
				static const unsigned long long mask = 0x00f000f000f000f0uLL;
				ret |= mask & B << SIZE & b >> SIZE;
			}
			{
				static const unsigned long long mask = 0x0f000f000f000f00uLL;
				ret |= mask & b << 2 * SIZE & B >> SIZE;
			}
			{
				static const unsigned long long mask = 0xf000f000f000f000uLL;
				ret |= mask & b << 3 * SIZE & B << SIZE;
			}
		}
		for (int i = SIZE * SIZE * 3; i < LINES_NUM; i++)
		{
			const unsigned long long b = LINES[i] & ~B;
			if ((b & -b) == b) ret |= b;
		}
		return ret;
	}

	inline enum Cell get_cell(int x, int y, int z) const
	{
		unsigned long long bit = BIT(x, y, z);
		if (Me & bit)return Cell::Me;
		else if (You & bit) return Cell::You;
		else return Cell::None;
	}

	int turn() const
	{
		return __builtin_popcountll(Me | You) + 1;
	}

	enum Color validate() const
	{
		assert(!(Me & You));
		const int d = __builtin_popcountll(Me) - __builtin_popcountll(You);
		assert(d == 0 || d == -1);
		return d == 0 ? Color::Black : Color::White;
	}

	enum Color player() const
	{
		return __builtin_parityll(Me | You) ? Color::White : Color::Black;
	}

	void print() const
	{
		enum Color now = validate();
		unsigned long long black, white;
		if (now == Color::Black) black = Me, white = You;
		else black = You, white = Me;
		cout << "    z=1  z=2  z=3  z=4\n";
		for (int y = SIZE - 1; y >= 0; y--)
		{
			cout << "y=" << y + 1 << " ";
			for (int z = 0; z < SIZE; z++)
			{
				for (int x = 0; x < SIZE; x++)
				{
					const unsigned long long bit = BIT(x, y, z);
					if (black & bit) cout << "\033[31mX\033[m";
					else if (white & bit) cout << "O";
					else cout << "-";
				}
				if (z + 1 < SIZE) cout << " ";
			}
			cout << "\n";
		}
		cout << "  x=1234 1234 1234 1234" << endl;
	}

	array<int, LINES_NUM> count() const
	{
		array<int, LINES_NUM> ret;
		for (int i = 0; i < LINES_NUM; i++)
		{
			if ((Me & LINES[i]) && (You & LINES[i])) ret[i] = 0;
			else if (Me & LINES[i]) ret[i] = __builtin_popcountll(Me & LINES[i]);
			else ret[i] = - __builtin_popcountll(You & LINES[i]);
		}
		return ret;
	}

	enum State place(int x, int y)
	{
		if (x < 0 || 4 <= x || y < 0 || 4 <= y)
		{
			cout << "out of range : (" << x + 1 << ", " << y + 1 << ")" << endl;
			return State::Invalid;
		}
		int z = 0;
		while (z < 4 && get_cell(x, y, z) != Cell::None) z++;
		if (z == 4)
		{
			cout << "row (" << x + 1 << ", " << y + 1 << ") is full" << endl;
			return State::Invalid;
		}
		Me |= BIT(x, y, z);
		swap(Me, You);
		return win(You);
	}

	enum State place_fast(unsigned long long bit)
	{
		Me |= bit;
		swap(Me, You);
		return win(You);
	}

	unsigned long long valid_move() const
	{
		return ((Me | You) << SIZE * SIZE | ((1uLL << SIZE * SIZE) - 1)) & ~(Me | You);
	}
};

struct Player
{
	bool verbose;
	int random;
	Player() : verbose(false), random(0) {}
	void set_verbose(bool verbose_) {verbose = verbose_;}
	void set_random(int random_) {random = random_;}
	pair<int, int> move_random(Board board)
	{
		unsigned long long hand = board.valid_move();
		assert(hand);

		const int sz = __builtin_popcountll(hand);
		int idx = rng() % sz + 1;
		int v = 0;
		for (; v < 64; v++) if (hand >> v & 1)
		{
			idx--;
			if (idx == 0) break;
		}
		assert(idx == 0);

		if (verbose) cout << "random move (" << X(v) + 1 << ", " << Y(v) + 1 << ", " << Z(v) + 1 << ")" << endl;
		return make_pair(X(v), Y(v));
	}
	virtual pair<int, int> move(Board board) = 0;
};

struct HumanPlayer : Player
{
	pair<int, int> move(Board board) override
	{
		enum Color now = board.validate();
		cout << endl;
		board.print();
		cout << "Input x y: place ";
		if (now == Color::Black) cout << "\033[31mX\033[m";
		else cout << "O";
		cout << " to (x, y)\t(1 <= x, y <= 4)"<<endl;
		int x, y;
		cin >> x >> y;
		return make_pair(x - 1, y - 1);
	}
};

struct Game
{
	Board board;
	Player* player1;
	Player* player2;
	vector<pair<int, int> > hand, start;
	bool verbose;

	Game(Player* p1, Player* p2, bool verbose=false, vector<pair<int, int> > start = {}) : player1(p1), player2(p2), verbose(verbose), start(start)
	{
		p1 -> set_verbose(verbose);
		p2 -> set_verbose(verbose);
	}

	enum State move(int turn)
	{
		enum State ret;
		while (true)
		{
			auto st = chrono::system_clock::now();
			pair<int, int> xy;
			if (turn < start.size()) xy = start[turn];
			else
			{
				Player* current_player = (turn % 2 == 0) ? player1 : player2;
				xy = current_player -> move(board);
			}
			ret = board.place(xy.first, xy.second);
			if (ret == State::Invalid)
			{
				cout << "Invalid move : (" << xy.first + 1 << ", " << xy.second + 1 << ")" << endl;
				continue;
			}
			hand.push_back(xy);
			if (verbose)
			{
				auto msec = chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - st);
				cout << "[turn " << turn + 1 << "] Place to (" << xy.first + 1 << ", " << xy.second + 1 << ") ";
				if (turn % 2 == 0) cout << "\033[31m(Black)\033[m";
				else cout << "(White)";
				cout << " by " << msec.count() / 1e3 << " sec" << endl;
				board.print();
			}
			break;
		}
		return ret;
	}

	enum Color game()
	{
		enum State ret;
		for (int turn = 0; turn < BOARD_SIZE; turn++)
		{
			ret = move(turn);
			if (ret == State::End) break;
		}
		board.print();
		enum Color result;
		if (ret == State::End)
		{
			cout << "END : winner is ";
			if (board.validate() == Color::White)
			{
				result = Color::Black;
				cout << "\033[31mBlack\033[m";
			}
			else
			{
				result = Color::White;
				cout << "White";
			}
			cout << " by " << hand.size() << " moves" << endl;
		}
		else
		{
			result = Color::Draw;
			cout << "DRAW" << endl;
		}
		cout << "hands :";
		for (auto [x,y] : hand) cout << " {" << x << ", " << y << "},";
		cout << endl;
		return result;
	}
};

template<typename F>
pair<unsigned long long, int> read_DFS(Board board, int level, const F& evaluate_func)
{
	if (level == 0) return make_pair(0uLL, evaluate_func(board));

	assert(level >= 1);
	unsigned long long hand = board.valid_move();
	if (!hand) return make_pair(0uLL, 0);

	{//reach
		{
			const unsigned long long r = hand & Board::reach(board.Me);
			if (r) return make_pair(r, INF);
		}
		{
			const unsigned long long r = hand & Board::reach(board.You);
			if (r) hand = r;
		}
	}

	pair<unsigned long long, int> mv = make_pair(0uLL, -INF);
	while (hand)
	{
		const unsigned long long bit = hand & -hand;
		Board b = board;
		enum State r = b.place_fast(bit);
		assert(r == State::Continue);
		int ev = -read_DFS(b, level - 1, evaluate_func).second;
		if (ev > INF - BOARD_SIZE * 2) ev = ev - 1;
		else if (ev < -INF + BOARD_SIZE * 2) ev = ev + 1;
		if (mv.second < ev) mv = make_pair(0uLL, ev);
		if (mv.second == ev) mv.first |= bit;
		hand ^= bit;
	}
	assert(mv.first);
	return mv;
}

template<typename F>
struct AIPlayer : Player
{
	using Player::verbose;
	using Player::random;
	int level;
	F evaluate_func;
	AIPlayer(int level, F evaluate_func) : level(level), evaluate_func(evaluate_func) { assert(level >= 1); }

	int evaluate_board(Board board, int level, int alpha, int beta)
	{
		unsigned long long hand = board.valid_move();
		if (!hand) return 0;

		{//reach
			{
				const unsigned long long r = hand & Board::reach(board.Me);
				if (r) return INF - board.turn();
			}
			{
				const unsigned long long r = Board::reach(board.You);
				if (hand & r) hand = hand & r;
				else if (level <= 0) return evaluate_func(board);
				hand &= ~(r >> SIZE * SIZE);
				if (!hand) return -(INF - (board.turn() + 1));
			}
		}

		for (const unsigned long long mask: move_order)
		{
			unsigned long long h = hand & mask;
			hand ^= h;
			while (h)
			{
				const unsigned long long bit = h & -h;
				Board b = board;
				enum State r = b.place_fast(bit);
				assert(r == State::Continue);
				int ev = -evaluate_board(b, level - 1, -beta, -alpha);
				alpha = max(alpha, ev);
				if (alpha >= beta) break;
				h ^= bit;
			}
			if (alpha >= beta) break;
		}
		return alpha;
	}

	pair<int, int> move(Board board) override
	{
		unsigned long long hand = board.valid_move();
		assert(hand);

		{
			const unsigned long long r = hand & Board::reach(board.Me);
			if (r)
			{
				int v = __builtin_ctzll(r);
				return make_pair(X(v), Y(v));
			}
		}
		{
			const unsigned long long r = hand & Board::reach(board.You);
			if (r) hand = r;
		}

		if (random && rng() % 100 < random) return move_random(board);

		unsigned long long mv = 0uLL;
		int mx = -INF;

		for (const unsigned long long mask: move_order)
		{
			unsigned long long h = hand & mask;
			hand ^= h;
			while (h)
			{
				const unsigned long long bit = h & -h;
				Board b = board;
				enum State r = b.place_fast(bit);
				assert(r == State::Continue);
				int ev = -evaluate_board(b, level - 1, -INF, -mx + 1);
				if (mx < ev) mv = 0uLL, mx = ev;
				if (mx == ev) mv |= bit;
				h ^= bit;
			}
		}

		//const int turn = board.turn();// turn number (the number of stones = turn - 1)
		//auto[mv, ev] = read_DFS(board, turn >= 40 ? level : level, evaluate_func);
		enum Color now = board.validate();
		if (verbose)
		{
			if (now == Color::Black) cout<<"\033[31m";
			cout << "score = " << mx << " by";
			for (int xyz = 0; xyz < BOARD_SIZE; xyz++) if (mv & 1uLL << xyz)
			{
				cout << " (" << X(xyz) + 1 << ", " << Y(xyz) + 1 << ", " << Z(xyz) + 1 << "),";
			}
			if (now == Color::Black) cout<<"\033[m";
			cout<<endl;
		}
		assert(mv);
		{
			vector<pair<int, int> > XY;
			for (int xyz = 0; xyz < BOARD_SIZE; xyz++) if (mv & 1uLL << xyz) XY.emplace_back(X(xyz), Y(xyz));
			return XY[rng() % XY.size()];
		}
	}
};

template<typename F>
struct AIPlayer_minimax : Player
{
	using Player::verbose;
	int level;
	F evaluate_func;
	AIPlayer_minimax(int level, F evaluate_func) : level(level), evaluate_func(evaluate_func) { assert(level >= 1); }

	pair<int, int> move(Board board) override
	{
		unsigned long long hand = board.valid_move();
		assert(hand);

		{
			const unsigned long long r = hand & Board::reach(board.Me);
			if (r)
			{
				int v = __builtin_ctzll(r);
				return make_pair(X(v), Y(v));
			}
		}
		{
			const unsigned long long r = hand & Board::reach(board.You);
			if (r)
			{
				int v = __builtin_ctzll(r);
				return make_pair(X(v), Y(v));
			}
		}

		const int turn = board.turn();// turn number (the number of stones = turn - 1)
		auto[mv, ev] = read_DFS(board, turn >= 40 ? level : level, evaluate_func);
		enum Color now = board.validate();
		if (verbose)
		{
			if (now == Color::Black) cout<<"\033[31m";
			cout << "score = " << ev << " by";
			for (int xyz = 0; xyz < BOARD_SIZE; xyz++) if (mv & 1uLL << xyz)
			{
				cout << " (" << X(xyz) + 1 << ", " << Y(xyz) + 1 << ", " << Z(xyz) + 1 << "),";
			}
			if (now == Color::Black) cout<<"\033[m";
			cout<<endl;
		}
		assert(mv);
		{
			vector<pair<int, int> > XY;
			for (int xyz = 0; xyz < BOARD_SIZE; xyz++) if (mv & 1uLL << xyz) XY.emplace_back(X(xyz), Y(xyz));
			return XY[rng() % XY.size()];
		}
	}
};

int main()
{
	{
		vector<unsigned long long> lines;
		for (int x = 0; x < SIZE; x++) for (int y = 0; y < SIZE; y++)
		{
			unsigned long long line = 0uLL;
			for (int z = 0; z < SIZE; z++) line |= BIT(x, y, z);
			lines.push_back(line);
		}
		for (int y = 0; y < SIZE; y++) for (int z = 0; z < SIZE; z++)
		{
			unsigned long long line = 0uLL;
			for (int x = 0; x < SIZE; x++) line |= BIT(x, y, z);
			lines.push_back(line);
		}
		for (int z = 0; z < SIZE; z++) for (int x = 0; x < SIZE; x++)
		{
			unsigned long long line = 0uLL;
			for (int y = 0; y < SIZE; y++) line |= BIT(x, y, z);
			lines.push_back(line);
		}
		for (int x = 0; x < SIZE; x++)
		{
			unsigned long long line1 = 0uLL, line2 = 0uLL;
			for (int yz = 0; yz < SIZE; yz++)
			{
				line1 |= BIT(x, yz, yz);
				line2 |= BIT(x, yz, SIZE - 1 - yz);
			}
			lines.push_back(line1);
			lines.push_back(line2);
		}
		for (int y = 0; y < SIZE; y++)
		{
			unsigned long long line1 = 0uLL, line2 = 0uLL;
			for (int zx = 0; zx < SIZE; zx++)
			{
				line1 |= BIT(zx, y, zx);
				line2 |= BIT(zx, y, SIZE - 1 - zx);
			}
			lines.push_back(line1);
			lines.push_back(line2);
		}
		for (int z = 0; z < SIZE; z++)
		{
			unsigned long long line1 = 0uLL, line2 = 0uLL;
			for (int xy = 0; xy < SIZE; xy++)
			{
				line1 |= BIT(xy, xy, z);
				line2 |= BIT(xy, SIZE - 1 - xy, z);
			}
			lines.push_back(line1);
			lines.push_back(line2);
		}
		{
			unsigned long long line1 = 0uLL, line2 = 0uLL, line3 = 0uLL, line4 = 0uLL;
			for (int xyz = 0; xyz < SIZE; xyz++)
			{
				line1 |= BIT(xyz, xyz, xyz);
				line2 |= BIT(xyz, xyz, SIZE - 1 - xyz);
				line3 |= BIT(xyz, SIZE - 1 - xyz, xyz);
				line4 |= BIT(xyz, SIZE - 1 - xyz, SIZE - 1 - xyz);
			}
			lines.push_back(line1);
			lines.push_back(line2);
			lines.push_back(line3);
			lines.push_back(line4);
		}
		assert(lines.size() == LINES_NUM);
		for (int j = 0; j < LINES_NUM; j++)
		{
			LINES[j] = lines[j];
			assert(__builtin_popcountll(LINES[j]) == 4);
			assert(Board::win(LINES[j]) == State::End);
		}

		unsigned long long ret = 0uLL;
		auto dfs = [&](auto self, int id, unsigned long long B, int rest) -> void
		{
			if (id == 64)
			{
				if (Board::win(B) == State::Continue)
				{
					//ret |= Board::reach_naive(B);
					//ret |= Board::reach(B);
					//assert(Board::reach_naive(B) == Board::reach(B));
				}
			}
			else
			{
				self(self, id + 1, B, rest);
				if (rest > 0) self(self, id + 1, B | 1uLL << id, rest - 1);
			}
		};
		//validation
		//dfs(dfs, 0, 0uLL, 6);
	}
	{//validate move_order
		unsigned long long mv = 0uLL;
		for(unsigned long long mask: move_order) mv |= mask;
		assert(mv == 0xffffffffffffffffuLL);
	}

	auto evaluate = [](const Board &board) -> int
	{
		int sum = 0;
		static const int weight[9] = {0,-60,-56,-16,0,16,56,60,0};
		for (int v : board.count()) sum += weight[v + 4];
		return sum;
	};
	auto continuous = [](unsigned long long rMe, const unsigned long long rYou) -> int
	{
		rMe &= ~(rYou << SIZE * SIZE);
		return __builtin_popcountll(rMe & rMe << SIZE * SIZE) * 10000;
	};
	auto reach_layer = [&](const enum Color now, unsigned long long rMe, unsigned long long rYou, const unsigned long long hand) -> int
	{
		const unsigned long long rMe_tmp = rMe & ~(hand | rYou << SIZE * SIZE);
		const unsigned long long rYou_tmp = rYou & ~(hand | rMe << SIZE * SIZE);
		rMe = rMe_tmp, rYou = rYou_tmp;

		static const unsigned long long mask_2 = 0x00000000ffff0000uLL;
		static const unsigned long long mask_3 = 0x0000ffff00000000uLL;
		static const unsigned long long mask_4 = 0xffff000000000000uLL;

		const unsigned long long intersection_3 = (rMe & rYou) & mask_3;
		rMe ^= intersection_3;
		rYou ^= intersection_3;

		int sum = 0;
		if (now == Color::Black)
		{//first (black) player
			{//Me, first (black) player
				sum += __builtin_popcountll(rMe & mask_2) * 30;//2nd layer
				sum += __builtin_popcountll(rMe & mask_3) * 140;//3rd layer
				sum += __builtin_popcountll(rMe & mask_4) * 11;//4th layer
			}
			{//You, second (white) player
				sum -= __builtin_popcountll(rYou & mask_2) * 64;//2nd layer
				sum -= __builtin_popcountll(rYou & mask_3) * 200;//3rd layer
				sum -= __builtin_popcountll(rYou & mask_4) * 16;//4th layer
			}
			if (intersection_3)
			{//if there exists intersections of reaches on 3rd layer
				if (__builtin_parityll(intersection_3))
				{//odd, black = Me
					sum += 100;
				}
				else
				{//even, white = You
					sum -= 150;
				}
			}
		}
		else
		{//second (white) player
			{//Me, second (white) player
				sum += __builtin_popcountll(rMe & mask_2) * 48;//2nd layer
				sum += __builtin_popcountll(rMe & mask_3) * 140;//3rd layer
				sum += __builtin_popcountll(rMe & mask_4) * 11;//4th layer
			}
			{//You, first (black) player
				sum -= __builtin_popcountll(rYou & mask_2) * 64;//2nd layer
				sum -= __builtin_popcountll(rYou & mask_3) * 200;//3rd layer
				sum -= __builtin_popcountll(rYou & mask_4) * 16;//4th layer
			}
			if (intersection_3)
			{//if there exists intersections of reaches on 3rd layer
				if (__builtin_parityll(intersection_3))
				{//odd, black = You
					sum -= 100;
				}
				else
				{//even, white = Me
					sum += 140;
				}
			}
		}
		return sum;
	};
	auto evaluate_cont_layer_intersection = [&](const Board &board) -> int
	{
		const enum Color now = board.player();
		const unsigned long long rMe = Board::reach(board.Me) & ~board.You;
		const unsigned long long rYou = Board::reach(board.You) & ~board.Me;
		const unsigned long long hand = board.valid_move();
		return evaluate(board) + continuous(rMe, rYou) - continuous(rYou, rMe) + reach_layer(now, rMe, rYou, hand);
	};

	HumanPlayer H;
	AIPlayer p1(7, evaluate_cont_layer_intersection);
	AIPlayer p2(7, evaluate_cont_layer_intersection);
	//p2.set_random(10);
	Game game(&p1, &p2, true, {{0, 0}, {3, 3}, {0, 3}, {3, 0}});
	game.game();
	return 0;
	int cnt[3] = {};
	for (int t = 1; t <= 5000; t++)
	{
		cout << "Game #" << t << endl;
		Game game(&p1, &p2, false, {{0, 0}, {3, 3}, {0, 3}, {3, 0}});
		enum Color r = game.game();
		cnt[r]++;
		cout << "Black : " << cnt[Color::Black] << endl;
		cout << "White : " << cnt[Color::White] << endl;
		cout << " Draw : " << cnt[Color::Draw] << endl;
	}
	return 0;
}
