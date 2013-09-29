package com.sudoku;
public class SudokuSolver {
	private int[][] sudoku = new int[9][9];
	private boolean solvable;
	public SudokuSolver(int[][] sudoku){
		this.sudoku = sudoku;
		solvable = true;
	}
	public SudokuSolver(){
		solvable = true;
	}
	public void setPuzzle(int[] puzzle){
		for(int i=0; i<9;i++){
			for(int j=0; j<9; j++){
				sudoku[i][j]=puzzle[i*9+j];
			}
		}
	}
	public void setPuzzle(int[][] puzzle){
		sudoku = puzzle;
	}
	public boolean checkWorkable(){
		return checkWorkable(sudoku);
	}
	public int[] getSolution(){
		if(solvable){
			int[] temp = new int[81];
			for(int i=0; i<9;i++){
				for(int j=0; j<9; j++){
					temp[i*9+j]=sudoku[i][j];
				}
			}
			return temp;
		}
		return null;
	}
	
	public static boolean checkWorkable(int[][] puzzle){
		return true;
	}
	public boolean solve(){
		solvable = solve(0,0,sudoku);
		return  solvable; 
	}
	private boolean solve(int i, int j, int[][] cells) {
		if (i == 9) {
			i = 0;
			if (++j == 9){
				return true;
			}
		}
		if (cells[i][j] != 0){
			// skip filled cells
			return solve(i+1,j,cells);
		}
		for (int val = 1; val <= 9; ++val) {
			if (legal(i,j,val,cells)) {  
				cells[i][j] = val;       
				if (solve(i+1,j,cells))  {
					return true;
				}
			}
		}
		cells[i][j] = 0; // reset on backtrack
		return false;
	}

    public static boolean legal(int i, int j, int val, int[][] cells) {
    	for (int k = 0; k < 9; ++k){
    		// row
    		if (val == cells[k][j]){
        		return false;
    		}
    	}
    	    
    	for (int k = 0; k < 9; ++k){
    		// col
    	    if (val == cells[i][k]){
        		return false;
    	    }
    	}
    	
    	int boxRowOffset = (i / 3)*3;
    	int boxColOffset = (j / 3)*3;
    	
    	for (int k = 0; k < 3; ++k){
    		// box
    		 for (int m = 0; m < 3; ++m){
    			 if (val == cells[boxRowOffset+k][boxColOffset+m]){
    				 return false;
    			 }
    		 }
    	}
    	   
    	return true; // no violations, so it's legal
    }

}
