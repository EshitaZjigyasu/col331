# console.c Changes

## Overview
A tiny update to the console interrupt handler adds a comment hinting at waking up a waiting process when input has been buffered.

## Detailed Changes
- In `consoleintr`, where input is added to the buffer and `input.w` is updated, a comment was inserted:
  ```c
          if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
+          // call myproc with the buf
            input.w = input.e;
          }
  ```

## Impact
The new comment documents intended future behavior: notifying the process reading from the console when data arrives. It's a preparatory note and does not affect current execution.
