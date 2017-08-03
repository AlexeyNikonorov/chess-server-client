#ifndef CHESS_HPP
#define CHESS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <cstddef>

using namespace std;

class Point {
	int m_x;
	int m_y;
public:
	Point() {}
	Point(int x, int y) { m_x = x; m_y = y; }
	Point(const char *str) { from_string(str); }
	bool from_string(const char *str) {
		m_x = (int) ( 7 - (str[1] - '1') );
		m_y = (int) (str[0] - 'a');
		return in_range(); 
	}
	char *to_string() const {
		static char str[3]; str[2] = '\0';
		str[0] = (char) m_y + 'a';
		str[1] = (char) (7 - m_x) + '1';
		return str;
	}
	int x() const { return m_x; }
	int y() const { return m_y; }
	bool in_range() const { return (0 <= m_x && m_x <= 7) && (0 <= m_y && m_y <= 7); }
	int xabs() const { return m_x < 0 ? -m_x : m_x; }
	int yabs() const { return m_y < 0 ? -m_y : m_y; }
	Point abs() const { return Point( xabs(), yabs() ); }
	Point sign() const {
		Point res;
		if (m_x == 0) res.m_x = 0;
		else if (m_x < 0) res.m_x = -1;
		else res.m_x = 1; 
		if (m_y == 0) res.m_y = 0;
		else if (m_y < 0) res.m_y = -1;
		else res.m_y = 1; 
		return res;
	}
	Point operator-(const Point& p) const {
		Point res;
		res.m_x = m_x - p.m_x;
		res.m_y = m_y - p.m_y;
		return res;
	}
	Point operator+(const Point& p) const {
		Point res;
		res.m_x = m_x + p.m_x;
		res.m_y = m_y + p.m_y;
		return res;
	}
	Point& operator+=(const Point& p) {
		m_x += p.m_x;
		m_y += p.m_y;
		return *this;
	}
	bool operator<(const Point& p) const {
		return xabs() < p.xabs() && yabs() < p.yabs();
	}
	bool operator==(const Point& p) const {
		return m_x == p.m_x && m_y == p.m_y;
	}
	bool operator!=(const Point& p) const {
		return m_x != p.m_x || m_y != p.m_y;
	}
};

class Board;

class Piece {
protected:
	int		m_type;
	int		m_color;
	Point	m_pos;
public:
	enum { BLACK = 0, WHITE = 1 };
	enum { PAWN = 1, KNIGHT = 2, BISHOP = 4, ROOK = 8, QUEEN = 16, KING = 32 };
	Piece() {}
	Piece(int type, int color) { m_type = type; m_color = color; }
	virtual ~Piece() {}
	void set_pos(const Point& pos) { m_pos = pos; }
	virtual void move(const Point& pos) { m_pos = pos; }
	virtual int valid_move(const Point& pos, const Board *board)=0;
	int color() const { return m_color; }
	int type() const { return m_type; }
	Point pos() const { return m_pos; }
};

class Board {
protected:
	Piece *m_squares[8][8];
public:
	Board() {}
	
	void clear() {
		for (int i = 0; i < 8; ++i)
			for (int j = 0; j < 8; ++j)
				set(Point(i, j), NULL);
	}
	
	virtual Piece *set_piece(Piece *piece, const Point& pos) {
		Piece *old_piece = get(pos);
		set(pos, piece);
		if (piece != NULL) piece->set_pos(pos);
		return old_piece;
	}
	
	virtual Piece *move_piece(const Point& p1, const Point& p2) {
		Piece *piece1 = get(p1);
		Piece *piece2 = get(p2);
		set(p2, piece1);
		set(p1, NULL);
		piece1->move(p2);
		return piece2;
	}
	
	Piece *get(const Point& pos) const {
		Piece *piece = m_squares[pos.x()][pos.y()];
		if (piece != NULL && piece->pos() != pos) cout << "Error\n";
		return m_squares[pos.x()][pos.y()];
	}
	
	Piece *set(const Point& pos, Piece *piece) {
		Piece *old_piece = m_squares[pos.x()][pos.y()];
		m_squares[pos.x()][pos.y()] = piece;
		if (piece != NULL) piece->set_pos(pos);
		return old_piece;
	}
};

class Pawn : public Piece {
	bool m_moved;
	bool m_en_passant;
public:
	Pawn(int color) : Piece(PAWN, color) { m_moved = false; m_en_passant = false; }
	
	virtual void move(const Point& pos) {
		m_moved = true;
		m_pos = pos;
	}
	
	virtual int valid_move(const Point& point, const Board *board) {
		Point d = point - m_pos;
		int sign = (m_color == WHITE) ? -1 : 1;
		int status;
		if ( (m_color == WHITE && point.x() == 0) ||
				(m_color == BLACK && point.x() == 7) ) status = 4;
		else status = 1;
		if (d.y() == 0) {
			if (d.x() == sign) {
				return (board->get(point) == NULL) ? status : 0;
			} else if (d.x() == sign * 2 && !m_moved) {
				Point step = d.sign();
				return (board->get(point) == NULL && board->get(m_pos + step) == NULL) ? 2 : 0;
			}
		} else if ( (d.y() == 1 || d.y() == -1) && d.x() == sign ) {
			if (board->get(point) != NULL) return status;
			Piece *piece;
			if (m_color == WHITE && point.x() == 2)
				piece = board->get(point + Point(1, 0));
			else if (m_color == BLACK && point.x() == 5)
				piece = board->get(point + Point(-1, 0));
			else return 0;
			if (piece->type() == PAWN && static_cast<Pawn*>(piece)->m_en_passant)
				return 3;
			return 0;
		}
		return 0;
	}
	
	bool en_passant(bool b) { return m_en_passant = b; }
};

class Knight : public Piece {
public:
	Knight(int color) : Piece(KNIGHT, color) {}
	
	virtual int valid_move(const Point& point, const Board*) {
		Point d = (point - m_pos).abs();
		return ( d.x() == 2 && d.y() == 1 ) || ( d.x() == 1 && d.y() == 2 ) ? 1 : 0;
	}
};

class Bishop : public Piece {
public:
	Bishop(int color) : Piece(BISHOP, color) {}
	
	virtual int valid_move(const Point& point, const Board *board) {
		Point d = point - m_pos;
		if ( d.xabs() != d.yabs() ) return 0;
		Point step = d.sign();
		for (Point pos = m_pos + step; pos != point; pos += step)
			if (board->get(pos) != NULL) return 0;
		return 1;
	}
};

class Rook : public Piece {
	bool m_moved;
public:
	Rook(int color) : Piece(ROOK, color) { m_moved = false; }
	
	virtual void move(const Point& pos) {
		m_moved = true;
		m_pos = pos;
	}
	
	virtual int valid_move(const Point& point, const Board *board) {
		Point d = point - m_pos;
		if (d.x() != 0 && d.y() != 0) return 0;
		Point step = d.sign();
		for (Point pos = m_pos + step; pos != point; pos += step)
			if (board->get(pos) != NULL) return 0;
		return 1;
	}
	
	bool moved() const { return m_moved; }
};

class Queen : public Piece {
public:
	Queen(int color) : Piece(QUEEN, color) {}

	virtual int valid_move(const Point& point, const Board *board) {
		Point d = point - m_pos;
		if ( d.x() == 0 || d.y() == 0 || ( d.xabs() == d.yabs() ) ) {
			Point step = d.sign();
			for (Point pos = m_pos + step; pos != point; pos += step)
				if (board->get(pos) != NULL) return 0;
			return 1;
		}
		return 0;
	}
};

class King : public Piece {
	bool m_moved;
public:
	King(int color) : Piece(KING, color) { m_moved = false; }
	
	virtual void move(const Point& pos) {
		m_moved = true;
		m_pos = pos;
	}
	
	virtual int valid_move(const Point& point, const Board*) {
		Point dabs = (point - m_pos).abs();
		return (dabs.x() <= 1 && dabs.y() <= 1) ? 1 : 0;
	}
	
	bool moved() const { return m_moved; }
};

class Chess {
	friend ostream& operator<<(ostream& os, Chess& chess);
	Board*	m_board;
	int		m_turn;
	Pawn*	m_en_passant;
	Pawn*	m_to_promote;
	King*	m_kings[2];
public:
	enum { BLACK = 0, WHITE = 1 };
	enum { CASTLING_KINGSIDE = 1, CASTLING_QUEENSIDE = 2 };
	enum { ACCEPTED = 1, PROMOTION = 2,
			INVALID_FORMAT = -1,
			IDLE_MOVE = -2,
			NO_SUCH_PIECE = -3,
			NOT_IN_TURN = -4,
			SQUARE_OCCUPIED = -5,
			CHECK = -6,
			INVALID_MOVE = -7 };

	Chess() {}
	
	void setup() {
		m_board = new Board();
		m_board->clear();
			
		#define NEW_W_PIECE(_class, _i, _j) { \
			_class *_p = new _class (Piece::WHITE); \
			m_board->set(Point(_i, _j), _p); \
		}

		#define NEW_B_PIECE(_class, _i, _j) { \
			_class *_p = new _class (Piece::BLACK); \
			m_board->set(Point(_i, _j), _p); \
		}
		
		int i, j;
		i = 6;
		for (j = 0; j < 8; ++j)
			NEW_W_PIECE(Pawn, i, j);
		NEW_W_PIECE(Rook, 7, 0);
		NEW_W_PIECE(Rook, 7, 7);
		NEW_W_PIECE(Knight, 7, 1);
		NEW_W_PIECE(Knight, 7, 6);
		NEW_W_PIECE(Bishop, 7, 2);
		NEW_W_PIECE(Bishop, 7, 5);
		NEW_W_PIECE(Queen, 7, 3);
		NEW_W_PIECE(King, 7, 4);
		
		i = 1;
		for (j = 0; j < 8; ++j)
			NEW_B_PIECE(Pawn, i, j);
		NEW_B_PIECE(Rook, 0, 0);
		NEW_B_PIECE(Rook, 0, 7);
		NEW_B_PIECE(Knight, 0, 1);
		NEW_B_PIECE(Knight, 0, 6);
		NEW_B_PIECE(Bishop, 0, 2);
		NEW_B_PIECE(Bishop, 0, 5);
		NEW_B_PIECE(Queen, 0, 3);
		NEW_B_PIECE(King, 0, 4);
		
		#undef NEW_W_PIECE
		#undef NEW_B_PIECE
		
		setup(m_board);
	}
	
	void setup(Board *board, int turn = WHITE) {
		m_board = board;
		m_turn = turn;
		m_en_passant = NULL;
		m_to_promote = NULL;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				Piece *piece = m_board->get(Point(i, j));
				if (piece != NULL && piece->type() == Piece::KING)
					m_kings[piece->color()] = static_cast<King*>(piece);
			}
		}
	}
	
	int enter_move(const char *str) {
		Point p1, p2;
		if ( p1.from_string(str) && p2.from_string(str + 2) ) {
			if (p1 == p2) return IDLE_MOVE;
			Piece *piece1 = m_board->get(p1);
			if (piece1 == NULL) return NO_SUCH_PIECE;
			if (piece1->color() != m_turn) return NOT_IN_TURN;
			Piece *piece2 = m_board->get(p2);
			if ( piece2 != NULL && piece2->color() == piece1->color() ) return SQUARE_OCCUPIED;
			if ( check(p1, p2) ) return CHECK;
			int status = piece1->valid_move(p2, m_board);
			if (status == 0) {
				return INVALID_MOVE;
			} else if (status == 1) {
				m_board->move_piece(p1, p2);
				delete piece2;
				switchTurn();
				return ACCEPTED;
			} else if (status == 2) {
				switchTurn();
				m_en_passant = static_cast<Pawn*>(piece1);
				m_en_passant->en_passant(true);
				m_board->move_piece(p1, p2);
				delete piece2;
				return ACCEPTED;
			} else if (status == 3) {
				m_board->set_piece( NULL, m_en_passant->pos() );
				delete m_en_passant;
				m_en_passant = NULL;
				m_board->move_piece(p1, p2);
				switchTurn();
				return ACCEPTED;
			} else if (status == 4) {
				m_to_promote = static_cast<Pawn*>(piece1);
				m_board->move_piece(p1, p2);
				delete piece2;
				return PROMOTION;
			}
		} else if (string(str) == "O-O") {
			int castling_status = handle_castling(CASTLING_KINGSIDE);
			if (castling_status == ACCEPTED) switchTurn();
			return castling_status;
		} else if (string(str) == "O-O-O") {
			int castling_status = handle_castling(CASTLING_QUEENSIDE);
			if (castling_status == ACCEPTED) switchTurn();
			return castling_status;
		} else if (str[0] == '=') {
			int promotion_status = handle_promotion(str);
			if (promotion_status == ACCEPTED) switchTurn();
			return promotion_status;
		}
		return INVALID_MOVE;
	}
	
	int check(const Point& p1, const Point& p2) {
		Piece *piece1 = m_board->get(p1);
		Piece *piece2 = m_board->get(p2);
		m_board->set(p1, NULL);
		m_board->set(p2, piece1);
		King *king = m_kings[m_turn];
		int type = under_attack( king->pos(), king->color() );
		m_board->set(p1, piece1);
		m_board->set(p2, piece2);
		return type;
	}
	
	int handle_castling(int side) {
		Point rook_position, king_position;
		if (m_turn == WHITE) {
			king_position = Point("e1");
			rook_position = (side == CASTLING_QUEENSIDE) ? Point("a1") : Point("h1");
		} else {
			king_position = Point("e8");
			rook_position = (side == CASTLING_QUEENSIDE) ? Point("a8") : Point("h8");
		}
		Piece *king = m_board->get(king_position);
		Piece *rook = m_board->get(rook_position);
		if (king == NULL || rook == NULL) return NO_SUCH_PIECE;
		if (king->type() != Piece::KING || rook->type() != Piece::ROOK) return NO_SUCH_PIECE;
		if ( static_cast<King*>(king)->moved() || static_cast<Rook*>(rook)->moved() ) return INVALID_MOVE;
		if ( !rook->valid_move(king_position, m_board) ) return SQUARE_OCCUPIED;
		Point new_king_position, new_rook_position, step;
		if (side == CASTLING_QUEENSIDE) {
			new_rook_position = rook_position + Point(0, 3);
			new_king_position = king_position + Point(0, -2);
			step = Point(0, -1);
		} else {
			new_rook_position = rook_position + Point(0, -2);
			new_king_position = king_position + Point(0, 2);
			step = Point(0, 1);
		}
		for (Point pos = king_position; pos != new_king_position; pos += step)
			if ( under_attack(pos, king->color()) ) return CHECK;
		if ( under_attack(new_king_position, king->color()) ) return CHECK;
		m_board->move_piece(king_position, new_king_position);
		m_board->move_piece(rook_position, new_rook_position);
		return ACCEPTED;
	}
	
	int under_attack(const Point& pos, int color) {
		static Point white_pawn_moves[2] = {
			Point(-1, 1), Point(-1, -1)
		};
		static Point black_pawn_moves[2] = {
			Point(1, 1), Point(1, -1)
		};
		static Point bishop_moves[4] = {
			Point(1, 1), Point(1, -1), Point(1, -1), Point(-1, -1)
		};
		static Point rook_moves[4] = {
			Point(0, 1), Point(0, -1), Point(1, 0), Point(-1, 0)
		};
		static Point knight_moves[8] = {
			Point(1, 2), Point(1, -2), Point(-1, 2), Point(-1, -2),
			Point(2, 1), Point(2, -1), Point(-2, 1), Point(-2, -1)
		};
		
		int type = 0;
		
		for (int i = 0; i < 4; ++i) {
			Point& step = bishop_moves[i];
			for (Point p = pos + step; p.in_range(); p += step) {
				Piece *piece = m_board->get(p);
				if (piece == NULL) continue;
				if (piece->color() == color) break;
				if ( piece->type() & (Piece::BISHOP | Piece::QUEEN) )
					return type |= piece->type();
				break;
			}
		}
		
		for (int i = 0; i < 4; ++i) {
			Point& step = rook_moves[i];
			for (Point p = pos + step; p.in_range(); p += step) {
				Piece *piece = m_board->get(p);
				if (piece == NULL) continue;
				if (piece->color() == color) break;
				if ( piece->type() & (Piece::ROOK | Piece::QUEEN) )
					return type |= piece->type();
				break;
			}
		}
		
		for (int i = 0; i < 8; ++i) {
			Point p = pos + knight_moves[i];
			if ( !p.in_range() ) continue;
			Piece *piece = m_board->get(p);
			if (piece == NULL) continue;
			if ( piece->color() != color && (piece->type() & Piece::KNIGHT) )
				return type |= Piece::KNIGHT;
		}
		
		Point *moves = (color == Piece::WHITE) ? white_pawn_moves : black_pawn_moves;
		for (int i = 0; i < 2; ++i) {
			Point p = pos + moves[i];
			if ( !p.in_range() ) continue;
			Piece *piece = m_board->get(p);
			if (piece == NULL) continue;
			if ( piece->color() != color && (piece->type() & Piece::PAWN) )
				return type |= Piece::PAWN;
		}
		
		return type;
	}
	
	int handle_promotion(const char *str) {
		if (m_to_promote == NULL) return INVALID_MOVE;
		Piece *promoted;
		switch (str[1]) {
			case 'N':
				promoted = new Knight(m_to_promote->color());
				break;
			case 'B':
				promoted = new Bishop(m_to_promote->color());
				break;
			case 'R':
				promoted = new Rook(m_to_promote->color());
				break;
			case 'Q':
				promoted = new Queen(m_to_promote->color());
				break;
			default:
				return INVALID_FORMAT;
		}
		m_board->set_piece(promoted, m_to_promote->pos());
		delete m_to_promote;
		m_to_promote = NULL;
		return ACCEPTED;
	}
	
	int switchTurn() {
		if (m_en_passant != NULL) {
			m_en_passant->en_passant(false);
			m_en_passant = NULL;
		}
		return m_turn ^= (WHITE ^ BLACK);
	}
	
	int turn() const { return m_turn; }
	const Board& board() const { return *m_board; }
};

/*void to_string(Piece *piece, char *str) {
	if (piece == NULL) {
		str[0] = '['; str[1] = ']';
		return;
	}
	str[0] = piece->color() == Piece::WHITE ? 'w' : 'b';
	switch ( piece->type() ) {
		case Piece::PAWN: str[1] = 'P'; break;
		case Piece::KNIGHT: str[1] = 'N'; break;
		case Piece::BISHOP: str[1] = 'B'; break;
		case Piece::ROOK: str[1] = 'R'; break;
		case Piece::QUEEN: str[1] = 'Q'; break;
		case Piece::KING: str[1] = 'K'; break;
	}
}

ostream& operator<<(ostream& os, Chess& chess) {
	for (int i = 0; i < 8; ++i) {
		os << 8 - i << "| ";
		for (int j = 0; j < 8; ++j) {
			char str[3]; str[2] = '\0';
			to_string( chess.m_board->get(Point(i, j)), str );
			os << str << " ";
		}
		os << "\n";
	}
	os << " |________________________\n   ";
	for (int j = 0; j < 8; ++j)
		os << (char)((char)(j) + 'a') << "  ";
	os << '\n';
	return os;
}*/

#endif
