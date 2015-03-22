#include <string>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include "Constants.h"
#include "Coordinate.h"
#include "Board.h"
#include "Bishop.h"
#include "Queen.h"
#include "King.h"
#include "Pawn.h"
#include "Knight.h"
#include "Rook.h"
#include "HumanPlayer.h"
#include "ComputerPlayer.h"
#include "algorithm/Node.h"
#include "algorithm/Distribution.h"
using namespace std;

/*This class mainly relate with interaction between board display and pieceschanges under different moves*/

Board::Board(const int n,vector<Piece *> p){
	pieces = p; /* list that contains pointers of pieces on the board */
	passant = new Board::EnPassant(); /* struct { Piece * and step} */
	steps = 0;/* record how many steps has been so far */
}


/*destructor , free memory of piece and passant*/
Board::~Board(){
	for(vector<Piece*>::iterator it=pieces.begin();it<pieces.end();it++)
	{
		delete (*it);
	}
	delete passant;

}

Board::Board(){
	pieces = vector<Piece*>(0);
	passant = new Board::EnPassant();

}

Board::Board(Board &b){
	steps = b.currentStep();
        vector<Piece*> &piecesB = b.getPieces();
	pieces = vector<Piece*>(0);
	for(vector<Piece*>::iterator it=piecesB.begin();it<piecesB.end();it++){
		pieces.push_back((*it)->copy());
		
	}
	this->freshBoard();
	passant = new Board::EnPassant();
	if(b.getPassant()->p!=NULL && b.getPassant()->step==steps){
		passant->p = this->getPiece(b.getPassant()->p->getPosition());
		passant->step = b.getPassant()->step;
	}

}

/*reserved function for debugging*/
void Board::record(int *from,int *to){
	char transaction[] = {from[1]+'a','1'+from[0],to[1]+'a',to[0]+'1','\n'};
	ofstream rec;
	rec.open("record",ios::app);
	rec << transaction[0];
	rec << transaction[1];
	rec << transaction[2];
	rec << transaction[3];
	rec << transaction[4];
	rec.close();
}

/*when variable pieces has changed, then call this function to update variable board*/
void Board::freshBoard(){
	int i,j;
	int l  = 8;
	/*initialize board*/
	for(i=0;i<8;i++)
		for(j=0;j<8;j++)
			board[i][j] = NULL;
	
	for(i=0;i<pieces.size();i++){
		Coordinate p = pieces.at(i)->getPosition();
		board[p[0]][p[1]] = pieces.at(i); 

	}
	
}
/*currently text mode display */
void Board::display(){
	using namespace std;
	int i,j;
	int l  = 8;
        /*display*/
	for(i=l-1;i>-1;i--){
		for(j=0;j<l;j++){
			string output = "";
			if(board[i][j]==NULL)
				output = "X  ";
			else if(board[i][j]->getColor() == BLACK){
				string black  = "B_";
				output = black + board[i][j]->getIdentifier();
			}
			else{
				string white = "W_";
				output = white +  board[i][j]->getIdentifier();
			}
			cout << output << " ";
		}	
			cout << endl;
		}
		cout << endl << endl;

}

void Board::initialize(){
	//instantiate 32 piecess
      	int i=0,j=0;
	
      	pieces.push_back(new Rook(WHITE,Coordinate()));
     	pieces.push_back(new Knight(WHITE,Coordinate()));
	pieces.push_back(new  Bishop(WHITE,Coordinate()));
	pieces.push_back(new Queen(WHITE,Coordinate()));
        pieces.push_back(new King(WHITE,Coordinate()));
	pieces.push_back(new Bishop(WHITE,Coordinate()));
     	pieces.push_back(new Knight(WHITE,Coordinate()));
      	pieces.push_back(new Rook(WHITE,Coordinate()));
        for(;j<16;j++){
		if(j<8)
			pieces.push_back(new Pawn(WHITE,Coordinate()));
		else
			pieces.push_back(new Pawn(BLACK,Coordinate()));
     	 }
	
      	pieces.push_back(new Rook(BLACK,Coordinate()));
     	pieces.push_back(new Knight(BLACK,Coordinate()));
	pieces.push_back(new Bishop(BLACK,Coordinate()));
	pieces.push_back(new Queen(BLACK,Coordinate()));
	pieces.push_back(new King(BLACK,Coordinate()));
	pieces.push_back(new Bishop(BLACK,Coordinate()));
     	pieces.push_back(new Knight(BLACK,Coordinate()));
      	pieces.push_back(new Rook(BLACK,Coordinate()));
	
	/*set the position of these piecess */
        for(i=0;i<16;i++){
	       	pieces.at(i)->setPosition(Coordinate(i/8,i%8));
	}
	for(i=48;i<64;i++){
		pieces.at(i-32)->setPosition(Coordinate(i/8,i%8)); 
	}

	/*update variable board*/
	this->freshBoard();
}

/*whether a borad sqaure is empty or occupied by a piece */
bool Board::occupied(const Coordinate position) const{
	if(board==NULL)
		exit(-1) ;
	if(board[position[0]][position[1]]==NULL)
		return FALSE;
	else
		return TRUE;
}

/* if a move is legal then make a move, return true */
bool Board::makeMove(const Coordinate from,const  Coordinate to){
	Piece * p = board[from[0]][from[1]];
	if(p!=NULL){
		if(p->makeMove(to,*this)){
		/*if is a capture move,then list pieces and variable need to be updated */
			if(this->getPiece(to)!=NULL)
				this->capture(from,to);
			this->increaseSteps();
			this->freshBoard();
			return TRUE;
			}
		}
	return FALSE;

}

/*update variable board and delete target piece from list pieces and also
from memory of that piece*/
void Board::capture(const Coordinate from,const Coordinate to){
	Piece * p = this->getPiece(to);
	for(int i=0;i<pieces.size();i++){
		if(pieces.at(i)==p){
			/*free memory,decremental variable numberOfPieces*/
			Piece * temp = pieces.at(i);
			pieces.erase(pieces.begin()+i);
			delete temp;
			break;
		}
			
	}

}


/*check whether a positi:on is under attack*/
bool Board::isUnderAttack(const Coordinate cord,const char color) const{
	bool isUnderAttack = FALSE;
	for(int i=0;i<pieces.size();i++){
		if(isUnderAttack)
			break;
		Piece* opponent=pieces.at(i);
		if(opponent->getColor()!=color){
			vector<Coordinate> moves;
			if(opponent->getIdentifier()==KING)
				moves = ((King*)opponent)->legalMovesWithoutCastling(*this);
			else
				moves = opponent->legalMoves(*this);
			for(int j=0;j<moves.size();j++){
				if(cord==moves.at(j)){
					isUnderAttack = TRUE;
					break;
				}
			}
		}
	}
	return isUnderAttack;
}
/*set and get methods for variable*/
Piece * Board::getPiece(const Coordinate position) const{
	
	return board[position[0]][position[1]];

}
vector<Piece*> & Board::getPieces()  {
	return pieces;
}
/*delete the original pawn and replace it with the piece that want to be promoted to. Currently just directly promote a pawn to a queen, more clever work need be done*/
void Board::doPromotion(Piece * p){
	for(int i=0;i<pieces.size();i++){
		if(pieces.at(i)== p){
			Coordinate cord = p->getPosition();
			pieces.at(i)= new Queen(p->getColor(),cord);
			delete p;
			break;
		}
	}

}

vector<Board*> Board::nextBoardStates(char color){
	std::vector<Board*> states(0);
	for(vector<Piece*>::iterator it=this->pieces.begin();it<this->pieces.end();it++){
		if((*it)->getColor()!=color)
			continue;

		vector<Coordinate> moves = (*it)->legalMoves(*this);
		for(vector<Coordinate>::iterator iit=moves.begin();iit<moves.end();iit++){
			Board* boardBackup  = new Board();
			*boardBackup = (*this);
			if(boardBackup->makeMove((*it)->getPosition(),(*iit))&& !Player::amICheckmated((*boardBackup),color)){
				states.push_back(boardBackup);
				}
		}

	}
	return states;
}

Board &Board::operator=(Board &b){
	steps = b.currentStep();
        vector<Piece*> &piecesB = b.getPieces();

	/*free old one*/
	for(vector<Piece*>::iterator it=pieces.begin();it<pieces.end();it++){
		delete (*it);
	}
	
	pieces.clear();
	/*assign new one*/
	for(vector<Piece*>::iterator it=piecesB.begin();it<piecesB.end();it++){
		pieces.push_back((*it)->copy());
		
	}
	
	this->freshBoard();

	passant->p = b.getPassant()->p;
	passant->step = b.getPassant()->step;
	
	return *this;
}

Board::EnPassant * Board::getPassant() const{
	return passant;
}



/*since when call this function, the function increaseSteps() has not bee called so that a +1 is needed*/
void Board::setPassant(Piece * p){
	passant->p = p;
	passant->step = this->currentStep()+1;	
}

int Board::currentStep() const{
	return steps;
}

void Board::increaseSteps(){
	steps += 1;
}

/*test purpose*/
int main(){
	Board start(32,vector<Piece*>(0));
	start.initialize();
	start.display();
	

	HumanPlayer human1(BLACK);
	HumanPlayer human2(WHITE);
	ComputerPlayer computer1(BLACK,'A');
	ComputerPlayer computer2(WHITE,'A');
	
	Distribution d(1,2);

 	while(1){
		//human1.play(start);
		//start.display();
		//human2.play(start);
		//start.display();
		computer1.play(start);
		start.display();
		computer2.play(start);
		start.display();
	}

}