package com.example.tictactoe;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.SwingUtilities;
import java.awt.Font;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class TicTacToeGUI extends JFrame {
    private JButton[][] buttons;
    private TicTacToeGame game;

    public TicTacToeGUI() {
        game = new TicTacToeGame();
        initializeUI();
    }

    private void initializeUI() {
        setTitle("Tic Tac Toe");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new GridLayout(3, 3));

        buttons = new JButton[3][3];
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                buttons[i][j] = new JButton();
                buttons[i][j].setFont(new Font("Arial", Font.PLAIN, 80)); // Adjust font size as needed
                buttons[i][j].addActionListener(new ButtonClickListener(i, j));
                add(buttons[i][j]);
            }
        }

        setSize(4000, 4000); // Set the size to 10 times bigger (4000x4000 pixels)
        setLocationRelativeTo(null); // Center the frame on the screen
        setVisible(true);
    }

    public class ButtonClickListener implements ActionListener {
        private int row;
        private int col;

        public ButtonClickListener(int row, int col) {
            this.row = row;
            this.col = col;
        }

        @Override
        public void actionPerformed(ActionEvent e) {
            if (game.makeMove(row, col)) {
                buttons[row][col].setText(String.valueOf(game.getCurrentPlayer()));
                if (game.checkWin()) {
                    JOptionPane.showMessageDialog(TicTacToeGUI.this, "Player " + game.getCurrentPlayer() + " wins!");
                    resetGame();
                } else if (game.isBoardFull()) {
                    JOptionPane.showMessageDialog(TicTacToeGUI.this, "It's a draw!");
                    resetGame();
                } else {
                    game.setCurrentPlayer(game.getCurrentPlayer() == 'X' ? 'O' : 'X');
                }
            } else {
                JOptionPane.showMessageDialog(TicTacToeGUI.this, "Invalid move. Please try again.");
            }
        }
    }

    private void resetGame() {
        game = new TicTacToeGame();
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                buttons[i][j].setText("");
            }
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new TicTacToeGUI());
    }
}
