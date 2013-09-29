package com.sudoku;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import com.sudoku.gui.FolderListActivity;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;

public class OpenMenu extends Activity implements OnClickListener {
	
	private static final String TAG ="Sudoku Grabber";
	@Override
	public void onCreate(Bundle savedInstanceState){
		super.onCreate(savedInstanceState);
		setContentView(R.layout.open_menu);
		
		View grabSudokuButton = findViewById(R.id.grab_sudoku_button);
		grabSudokuButton.setOnClickListener(this);
		
		View playSudokuButton = findViewById(R.id.play_sudoku_button);
		playSudokuButton.setOnClickListener(this);
		
		View aboutButton = findViewById(R.id.about_button);
		aboutButton.setOnClickListener(this);
		
		View exitButton = findViewById(R.id.exit_button);
		exitButton.setOnClickListener(this);
	}

	@Override
	public void onClick(View v) {
		switch(v.getId()){
		case R.id.about_button:
			Intent aboutAct = new Intent(this,AboutInfo.class);
			startActivity(aboutAct);
			break;
		case R.id.grab_sudoku_button:
			checkSample();
			Intent grabSudokuAct = new Intent(this, SudokuSelector.class);
			startActivity(grabSudokuAct);
			break;
		case R.id.play_sudoku_button:
			Intent gameAct = new Intent(this,FolderListActivity.class);
			startActivity(gameAct);
			break;
		case R.id.exit_button:
			this.finish();
			break;
		default:
			break;
		}
		
	}
	private void checkSample(){
		String dataPath = getResources().getString(R.string.sample_data_path);
		String labelPath = getResources().getString(R.string.sample_label_path);
		String dataName = getResources().getString(R.string.sample_data_name);
		String labelName = getResources().getString(R.string.sample_label_name);
		FileInputStream fi;
		try {
			 fi= new FileInputStream(dataPath);
		} catch (IOException e) {
			try {
				InputStream bf = getAssets().open(dataName);
				FileOutputStream fo = new FileOutputStream(dataPath);
				byte[] buf = new byte[1024];
				int len = 0;
				while((len = bf.read(buf))>0){
					fo.write(buf,0,len);
				}
				bf.close();
				fo.close();
			} catch (IOException e1) {
				e1.printStackTrace();
			}
		}
		try {
			fi = new FileInputStream(labelPath);
		} catch (IOException e) {
			try {
				InputStream bf = getAssets().open(labelName);
				FileOutputStream fo = new FileOutputStream(labelPath);
				byte[] buf = new byte[1024];
				int len = 0;
				while((len = bf.read(buf))>0){
					fo.write(buf,0,len);
				}
				bf.close();
				fo.close();
			} catch (IOException e1) {
				e1.printStackTrace();
			}
		}
	}
}
