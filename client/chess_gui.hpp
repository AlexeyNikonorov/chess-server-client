#ifndef CHESS_GUI_HPP
#define CHESS_GUI_HPP

#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>
#include <QSvgWidget>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QString>
#include <QMouseEvent>
#include <QTcpSocket>
#include "../chess.hpp"

class PieceWidget : public QSvgWidget {
public:
	PieceWidget(QWidget *parent=0) : QSvgWidget(parent) {}
	
	void moveCenter(const QPoint& pos) {
		QRect rect = geometry();
		rect.moveCenter(pos);
		setGeometry(rect);
	}
};

class PawnWidget : public PieceWidget, public Pawn {
public:
	PawnWidget(int color, QWidget *parent=0) : PieceWidget(parent), Pawn(color) {
		if (color == Piece::WHITE) load( QString("img/wp.svg") );
		else load( QString("img/bp.svg") );
	}
};

class KnightWidget : public PieceWidget, public Knight {
public:
	KnightWidget(int color, QWidget *parent=0) : PieceWidget(parent), Knight(color) {
		if (color == Piece::WHITE) load( QString("img/wn.svg") );
		else load( QString("img/bn.svg") );
	}
};

class BishopWidget : public PieceWidget, public Bishop {
public:
	BishopWidget(int color, QWidget *parent=0) : PieceWidget(parent), Bishop(color) {
		if (color == Piece::WHITE) load( QString("img/wb.svg") );
		else load( QString("img/bb.svg") );
	}
};

class RookWidget : public PieceWidget, public Rook {
public:
	RookWidget(int color, QWidget *parent=0) : PieceWidget(parent), Rook(color) {
		if (color == Piece::WHITE) load( QString("img/wr.svg") );
		else load( QString("img/br.svg") );
	}
};

class QueenWidget : public PieceWidget, public Queen {
public:
	QueenWidget(int color, QWidget *parent=0) : PieceWidget(parent), Queen(color) {
		if (color == Piece::WHITE) load( QString("img/wq.svg") );
		else load( QString("img/bq.svg") );
	}
};

class KingWidget : public PieceWidget, public King {
public:
	KingWidget(int color, QWidget *parent=0) : PieceWidget(parent), King(color) {
		if (color == Piece::WHITE) load( QString("img/wk.svg") );
		else load( QString("img/bk.svg") );
	}
};

class SquareWidget : public QSvgWidget {
	PieceWidget *mPiece;

public:
	enum { BLACK = 0, WHITE = 1 };

	SquareWidget(int color, QWidget *parent=0) : QSvgWidget(parent) {
		mPiece = nullptr;
		if (color == WHITE) load( QString("img/sq_w.svg") );
		else if (color == BLACK) load( QString("img/sq_b.svg") );
	}
	
	PieceWidget *replacePiece(PieceWidget *piece) {
		if (piece != nullptr) piece->QWidget::move( pos() );
		PieceWidget *copy = mPiece;
		mPiece = piece;
		return copy;
	}
	
	PieceWidget *piece() { return mPiece; }
	static int switchColor(int color) { return WHITE ^ BLACK ^ color; }
};

class BoardWidget : public QWidget, public Board {
	Q_OBJECT;
	int				mSquareSize;
	SquareWidget*	mSquares[8][8];
	PieceWidget*	mActivePiece;
	SquareWidget*	mStartingSquare;
	QPoint			mStartingPoint;
	Chess			mChess;
	QTcpSocket*		mSocket;
	QString			mLastMove;

public:
	BoardWidget(QWidget *parent=0) : QWidget(parent), Board() {
		mActivePiece = nullptr;
		mStartingSquare = nullptr;
		setSize();
		setSquares();
		setPieces();
		mSocket = new QTcpSocket();
		mSocket->connectToHost("127.0.0.1", 3000);
		connect(mSocket, SIGNAL(readyRead()), this, SLOT(listener()));
	}
	
	void setSize() {
		QRect rect = QApplication::desktop()->availableGeometry();
		QPoint center = rect.center();
		int boardSize = ( rect.width() < rect.height() ) ? rect.width() : rect.height();
		boardSize = (int) (boardSize * 0.65);
		mSquareSize = boardSize / 8;
		boardSize = mSquareSize * 8;
		rect.setSize( QSize(boardSize, boardSize) );
		rect.moveCenter(center);
		setGeometry(rect);
	}
	
	void setSquares() {
		QSize squareSize(mSquareSize, mSquareSize);
		QPoint pos(0, 0);
		int color = SquareWidget::WHITE;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				SquareWidget *square = new SquareWidget(color, this);
				square->resize(squareSize);
				square->move(pos);
				mSquares[i][j] = square;
				color = SquareWidget::switchColor(color);
				pos.rx() += mSquareSize;
			}
			color = SquareWidget::switchColor(color);
			pos.rx() = 0;
			pos.ry() += mSquareSize;
		}
	}
	
	void setPieces() {
		QSize size(mSquareSize, mSquareSize);
		Board::clear();

		#define NEW_W_PIECE(_class, _i, _j) { \
			_class *_p = new _class (Piece::WHITE, this); \
			_p->resize(size); \
			mSquares[_i][_j]->replacePiece(_p); \
			set_piece(_p, Point(_i, _j)); \
		}

		#define NEW_B_PIECE(_class, _i, _j) { \
			_class *_p = new _class (Piece::BLACK, this); \
			_p->resize(size); \
			mSquares[_i][_j]->replacePiece(_p); \
			set_piece(_p, Point(_i, _j)); \
		}

		int i, j;
		i = 6;
		for (j = 0; j < 8; ++j)
			NEW_W_PIECE(PawnWidget, i, j);
		NEW_W_PIECE(RookWidget, 7, 0);
		NEW_W_PIECE(RookWidget, 7, 7);
		NEW_W_PIECE(KnightWidget, 7, 1);
		NEW_W_PIECE(KnightWidget, 7, 6);
		NEW_W_PIECE(BishopWidget, 7, 2);
		NEW_W_PIECE(BishopWidget, 7, 5);
		NEW_W_PIECE(QueenWidget, 7, 3);
		NEW_W_PIECE(KingWidget, 7, 4);
		
		i = 1;
		for (j = 0; j < 8; ++j)
			NEW_B_PIECE(PawnWidget, i, j);
		NEW_B_PIECE(RookWidget, 0, 0);
		NEW_B_PIECE(RookWidget, 0, 7);
		NEW_B_PIECE(KnightWidget, 0, 1);
		NEW_B_PIECE(KnightWidget, 0, 6);
		NEW_B_PIECE(BishopWidget, 0, 2);
		NEW_B_PIECE(BishopWidget, 0, 5);
		NEW_B_PIECE(QueenWidget, 0, 3);
		NEW_B_PIECE(KingWidget, 0, 4);

		mChess.setup(this);
	}
	
	virtual void mousePressEvent(QMouseEvent *ev) {
		QPoint pos(ev->pos().x() / mSquareSize, ev->pos().y() / mSquareSize);
		SquareWidget *square = mSquares[pos.y()][pos.x()];
		PieceWidget *piece = square->piece();
		if (piece == nullptr) return;
		piece->raise();
		mActivePiece = piece;
		mStartingPoint = pos;
		mStartingSquare = square;
	}
	
	virtual void mouseMoveEvent(QMouseEvent *ev) {
		if (mActivePiece != nullptr) mActivePiece->moveCenter( ev->pos() );
	}
	
	virtual void mouseReleaseEvent(QMouseEvent *ev) {
		if (mActivePiece == nullptr) return;
		QPoint endPoint(ev->pos().x() / mSquareSize, ev->pos().y() / mSquareSize);
		const char *move = formMove(mStartingPoint, endPoint);
		/*int status = mChess.enter_move(move);
		if (status < 0) {
			cout << status << endl;
			mStartingSquare->replacePiece( mStartingSquare->piece() );
		}*/
		send(move);
		mActivePiece = nullptr;
	}
	
	virtual Piece *move_piece(const Point& p1, const Point& p2) {
		SquareWidget *start = mSquares[p1.x()][p1.y()];
		SquareWidget *end = mSquares[p2.x()][p2.y()];
		end->replacePiece( start->replacePiece(nullptr) );
		return Board::move_piece(p1, p2);
	}

	virtual Piece *set_piece(Piece *piece, const Point& pos) {
		if (piece == NULL)
			mSquares[pos.x()][pos.y()]->replacePiece(nullptr);
		return Board::set_piece(piece, pos);
	}
	
	static const char *formMove(const QPoint& start, const QPoint& end) {
		static const char *castlingRight = "O-O";
		static const char *castlingLeft = "O-O-O";
		static char move[6];
		
		if ( start.x() == 4 && (start.y() == 0 || start.y() == 7) ) {
			QPoint d = end - start;
			if (d.y() == 0) {
				if (d.x() == 2) return castlingRight;
				if (d.x() == -2) return castlingLeft;
			}
		}
		pointToString(start, move);
		pointToString(end, move + 2);
		move[4] = '\0';
		return move;
	}
	
	static void pointToString(const QPoint& point, char *str) {
		str[0] = (char) point.x() + 'a';
		str[1] = (char) (7 - point.y()) + '1';
	}

public slots:
	void listener() {
		char response[64];
		mSocket->readLine(response, 64);
		cout << "response " << response << endl;
		if ( mChess.enter_move(response) < 0) {
			if (mStartingSquare != nullptr) mStartingSquare->replacePiece( mStartingSquare->piece() );
		}
	}
	
	void send(const char *request) {
		mSocket->write(request);
	}
};

#endif
