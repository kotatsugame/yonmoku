import copy
import random
import time

LINE = []
for x in range(4):
	for y in range(4):
		LINE.append([(x, y, z) for z in range(4)])
for y in range(4):
	for z in range(4):
		LINE.append([(x, y, z) for x in range(4)])
for z in range(4):
	for x in range(4):
		LINE.append([(x, y, z) for y in range(4)])
for x in range(4):
	LINE.append([(x, yz, yz) for yz in range(4)])
	LINE.append([(x, 3 - yz, yz) for yz in range(4)])
for y in range(4):
	LINE.append([(zx, y, zx) for zx in range(4)])
	LINE.append([(3 - zx, y, zx) for zx in range(4)])
for z in range(4):
	LINE.append([(xy, xy, z) for xy in range(4)])
	LINE.append([(3 - xy, xy, z) for xy in range(4)])
LINE.append([(xyz, xyz, xyz) for xyz in range(4)])
LINE.append([(3 - xyz, xyz, xyz) for xyz in range(4)])
LINE.append([(xyz, 3 - xyz, xyz) for xyz in range(4)])
LINE.append([(3 - xyz, 3 - xyz, xyz) for xyz in range(4)])

IDX = [[16*z + 4*y + x for (x, y, z) in L] for L in LINE]

class Board:
	"""
		board: [0, 1, -1] * 64
		0 -> None
		1 -> Black
		-1 -> White
	"""
	def __init__(self):
		self.board = [0] * 64
	
	def print(self):
		print('   z=1  z=2  z=3  z=4')
		for y in reversed(range(4)):
			print(f'y={y+1}', end='')
			for z in range(4):
				for x in range(4):
					i = self.board[z*16 + y*4 + x]
					c = '-' if i == 0 else '\033[31mX\033[m' if i == 1 else 'O'
					print(c, end='')
				print(' ', end='')
			print()
		print(' x=1234 1234 1234 1234')

	def count(self):
		"""
			 1 -  4 -> Black
			-1 - -4 -> White
		"""
		ret = []
		for I in IDX:
			s = 0
			B = False
			W = False
			for i in I:
				c = self.board[i]
				s += c
				if c == 1: B = True
				elif c == -1: W = True
			if B and W: s = 0
			ret.append(s)
		return ret

	def win(self):
		r = self.count()
		if 4 in r:
			return 1 # Black win
		if -4 in r:
			return -1 # White win
		return 0 # None
	
	def place(self, x, y):
		if not (0 <= x < 4 and 0 <= y < 4):
			print(f'out of range : {(x+1, y+1)}')
			return None

		z = 0
		while z < 4 and self.board[z*16 + y*4 + x] != 0:
			z += 1
		if z == 4:
			# print(f'row {(x+1, y+1)} is full')
			return None
		
		if sum(self.board) % 2 == 0:
			# print(f'place Black at {(x+1, y+1, z+1)}')
			self.board[z*16 + y*4 + x] = 1 # Black
		else:
			# print(f'place White at {(x+1, y+1, z+1)}')
			self.board[z*16 + y*4 + x] = -1 # White
		return self.win()
	
	def valid_move(self):
		ret = []
		for x in range(4):
			for y in range(4):
				z = 0
				while z < 4 and self.board[z*16 + y*4 + x] != 0:
					z += 1
				if z < 4:
					ret.append((x, y, z))
		return ret

class Game:
	def __init__(self, player1, player2, *, verbose=False, first=False):
		self.board= Board()
		self.hand = []
		self.player1 = player1
		self.player2 = player2
		self.player1.set_mark('X', 1)
		self.player2.set_mark('O', -1)
		self.verbose = verbose
		self.first = first
	
	def move(self, turn):
		start = time.time()
		while True:
			if self.first and turn < 4:
				x, y = [(0, 0), (3, 3), (0, 3), (3, 0)][turn]
			else:
				x, y = self.player1.move(self.board)
			ret = self.board.place(x, y)
			if ret is None:
				print('Invalid move :', (x+1 ,y+1))
				continue
			self.hand.append((x+1, y+1))
			if self.verbose:
				T = time.time() - start
				if turn % 2 == 0:
					print(f'\033[31m[turn {turn+1}] Place to', (x+1, y+1), f'(Black) by {T:.2f} sec\033[m')
				else:
					print(f'[turn {turn+1}] Place to', (x+1, y+1), f'(White) by {T:.2f} sec')
				self.board.print()
			break
		self.player1, self.player2 = self.player2, self.player1
		return ret

	def game(self):
		for turn in range(64):
			ret = self.move(turn)
			if ret != 0:
				break

		self.board.print()
		if ret != 0:
			winner = 'Black' if ret == 1 else 'White'
			print('END : winner is', winner, 'by', len(self.hand), 'moves')
		else:
			print('DRAW')
		print('hand :', self.hand)
		return ret

class Player:
	def __init__(self):
		self.mark = None

	def set_mark(self, mark, sign):
		self.mark = mark

	def move(self, board):
		assert self.mark is not None

		while True:
			board.print()
			print('input x y: place', self.mark, 'to (x, y)    (1 <= x, y <= 4)')
			s = input()
			try:
				x, y = map(lambda s: int(s) - 1, s.split())
				return (x, y)
			except e:
				print('Invalid input :', s)
				continue

def evaluate(board, ret):
	return sum(ret)

DOWN = [[(z - 1)*16 + y*4 + x for (x, y, z) in L if z > 0] for L in LINE]

def evaluate_bonus(board, ret):
	ev = 0
	for (c, di) in zip(ret, DOWN):
		if c == 0: continue
		t = abs(c)
		if abs(c) == 3:
			if any(board.board[i] == 0 for i in di):
				t += 2
		ev += t if c > 0 else -t
	return ev

dict_bonus_cont = dict()
def evaluate_bonus_cont(board, ret):
	global dict_bonus_cont
	if board in dict_bonus_cont: return dict_bonus_cont[board]

	ev = 0
	win = []
	lose = []
	for (c, L, I, di) in zip(ret, LINE, IDX, DOWN):
		if c == 0: continue
		t = abs(c)
		if t == 3:
			if any(board.board[i] == 0 for i in di):
				t += 2
			emp = [p for p, i in zip(L, I) if board.board[i] == 0]
			assert len(emp) == 1
			if c > 0:
				win.append(emp[0])
			else:
				lose.append(emp[0])
		ev += t if c > 0 else -t
	win.sort()
	for (x0, y0, z0), (x1, y1, z1) in zip(win, win[1:]):
		if x0 == x1 and y0 == y1 and z0 + 1 == z1:
			ev += 100
	lose.sort()
	for (x0, y0, z0), (x1, y1, z1) in zip(lose, lose[1:]):
		if x0 == x1 and y0 == y1 and z0 + 1 == z1:
			ev -= 100
	dict_bonus_cont[board] = ev
	return ev

DIV_10 = [all(z == 4 for (x, y, z) in L) or len(set((x, y) for (x, y, z) in L)) == 1 for L in LINE]

dict_bonus_div10 = dict()
def evaluate_bonus_div10(board, ret):
	global dict_bonus_div10
	if board in dict_bonus_div10: return dict_bonus_div10[board]

	ev = 0
	for (c, di, d) in zip(ret, DOWN, DIV_10):
		if c == 0: continue
		t = abs(c)
		if t == 3:
			if any(board.board[i] == 0 for i in di):
				t += 2
		if d: t /= 10
		ev += t if c > 0 else -t
	dict_bonus_div10[board] = ev
	return ev

def read_DFS(board, sign, level, evaluate_func):
	ret = [c * sign for c in board.count()]
	if 4 in ret:
		return (None, 10**9)
	if -4 in ret:
		return (None, -10**9)
	
	if level == 0:
		return (None, evaluate_func(board, ret))

	assert level >= 1
	hand = board.valid_move()
	if not hand: return (None, 0)

	mv = []
	mx = -10**9
	for (x, y, z) in hand:
		b = copy.deepcopy(board)
		assert b.place(x, y) is not None
		_, ev = read_DFS(b, -sign, level - 1, evaluate_func)
		ev = -ev
		if mx < ev:
			mx = ev
			mv = []
		if mx == ev:
			mv.append((x, y, z))
	assert mv
	return mv, mx

class AI:
	"""
	Lv 0: random
	Lv 1: reach stop / win if reach
	Lv 2: eval
	Lv 2+n: read n turns
	"""
	def __init__(self, level, *, evaluate_func=None):
		self.level = level
		self.mark = None
		self.sign = None
		self.evaluate_func = evaluate_func

	def set_mark(self, mark, sign):
		self.mark = mark
		self.sign = sign
	
	def move(self, board):
		assert self.mark is not None
		assert self.sign is not None

		hand = board.valid_move()
		assert hand

		if self.level == 0:
			return random.choice(hand)[:2]

		ret = [c * self.sign for c in board.count()]
		assert 4 not in ret and -4 not in ret
		hand = set(hand)
		if 3 in ret:
			for (c, L) in zip(ret, LINE):
				can = list(set(L) & hand)
				if can and c == 3:
					return can[0][:2]
		if -3 in ret:
			for (c, L) in zip(ret, LINE):
				can = list(set(L) & hand)
				if can and c == -3:
					return can[0][:2]

		if self.level == 1:
			return random.choice(list(hand))[:2]

		if self.level == 2:
			mv = []
			mx = -4
			for (c, L) in zip(ret, LINE):
				can = list(set(L) & hand)
				if not can: continue
				if mx < c:
					mx = c
					mv = []
				if mx == c:
					mv.extend(can)
			assert mv
			return random.choice(mv)[:2]

		assert self.level >= 3
		#assert self.evaluate_func is not None
		#turn = sum(c != 0 for c in board.board[3*16:])
		#lv = 3 if turn < 8 else 5 if turn < 12 else 7
		lv = self.level - 2
		mv, ev = read_DFS(board, self.sign, lv, self.evaluate_func)
		if self.sign == 1:
			print('\033[31mscore =', ev, 'by', mv, '\033[m')
		else:
			print('score =', ev, 'by', mv)
		assert mv
		return random.choice(mv)[:2]



random.seed(114515)
#Game(Player(), AI(level=5, evaluate_func=evaluate_bonus), verbose=True).game()
#exit()

count = dict()
NUM = 100
for t in range(NUM):
	print(f'Game #{t+1}')
	P1 = AI(level=7, evaluate_func=evaluate_bonus)
	P2 = AI(level=7, evaluate_func=evaluate_bonus_cont)
	game = Game(P1, P2, verbose=True, first=True)
	r = game.game()
	count[r] = count.get(r, 0) + 1
	print('Black', count.get(1, 0))
	print('White', count.get(-1, 0))
	print('Draw', count.get(0, 0))
