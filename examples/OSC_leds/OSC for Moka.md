OSC for Moka
// To Moka

/moka

	/led

		/set

		/int

		/rgb

			/one
				/set/one 1 2 1			// Turn ON led x2 y1
				/int/one 3 3 4			// Set brightness to 3 on led x3 y4
				/rgb/one 56 12 			// Set color 56 to led at index 12

			/row
				/set/row 1 3 			// Turn ON row 3
				/rgb/row 3 2 1 4 		// Set color blue for 4 leds on row 2, sarting from col 1

			/col
				/int/col 2 0 			// Set brightness to 2 on first col
				/set/col 0 0 2 3 		// Turn OFF 3 leds on first col, starting from row 2

			/map
				/rgb/map 12 2 3 4 5 	// Set color green on a 4x5 pad, starting from pos 2, 3

			/all
				/all/rgb 3				// Set all led blue
				/all/rgb 52				// Set all led orange


	/key pos 							// Ask for key values by pos (/key x y state)

	/key index 							// Ask for key value by index (/key index state)

	/size								// Ask for board size

	/start 								// Display startup sequence



// From Moka

	/key x y s
		/key 0 0 1			// Key x0 y0 is pushed

	/key i s
		/key 12 0			// Key at index 12 is not pushed

	/size x y
		/size 12 8 			// Board is 12 columns x 8 rows
