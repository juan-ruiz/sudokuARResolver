package com.sudoku.test;

import com.sudoku.SudokuSolver;

public class SudokuSolverTest {
    public static void main(String[] args){
    	int[][] sudoku = {{0,0,9,0,0,0,0,0,0},{0,8,0,6,0,5,0,2,0},{5,0,1,0,7,8,0,0,0},
    			{0,0,0,0,0,0,7,0,0},{7,0,6,0,4,0,1,0,2},{0,0,4,0,0,0,0,0,0},
    			{0,0,0,7,2,0,9,0,3},{0,9,0,3,0,1,0,8,0},{0,0,0,0,0,0,6,0,0}};
    	SudokuSolver sv = new SudokuSolver(sudoku);
    	sv.solve();
    	for(int i=0; i<9;i++){
    		for(int j=0; j<0; j++){
    			System.out.print(sudoku[i][j]+" ");
    		}
    		System.out.println();
    	}
    }
}
