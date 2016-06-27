import java.io.*;
import java.util.*;

/**
 * Counts the number of factors of a specified number within a certain range.
 * 
 * @author Robert Reichel
 * @file Fcount.java
 */
public class Fcount {
  /**
   * A private class that represents a worker
   * Extends the thread class and can be run
   * and used to calculate the number of numbers with a given factor
   */
  private static class Worker extends Thread {
    /** List of numbers to peform the operation on */
    List<Long> numbers;
    /** The number of factors to look for */
    int fTarget;
    /** Public value to keep track of the count */
    public int count = 0;
    /**
     * The constructor for the Worker class. Just takes some parameters
     * and stores them
     * @param fTarget The number of factors we want to look for
     * @param stuffToUse The list of values to perform the operation on
     */
    public Worker(int fTarget, List<Long> stuffToUse) {
      this.fTarget = fTarget;
      this.numbers = stuffToUse;
    }
    /**
     * This method is what gets run when the thread actually starts up.
     * It calculates, using the previously provided values the number of
     * numbers in the list that have the fCount number of factors
     */
    public void run() {
      Iterator<Long> nums = numbers.iterator();
      while (nums.hasNext()) {
        int fcount = 0;
        long val = (long) nums.next();
        // Check factors up to the square root of the number we're
        // factoring.
        long f = 2;
        while (f * f < val) {
          if (val % f == 0)
            // Count both f and vList[ j ] / f as factors.
            fcount += 2;
          f++;
        }
        // If the number is a perfect square, it gets one more
        // factor.
        if (f * f == val)
          fcount++;

        // Count this value, if it has the desired number of factors.
        if (fcount == fTarget)
          count++;
      }
    }
  }
  /**
   * This method reads in values from standard in and puts them in a list.
   * It inputs the numbers as long
   * @return A list of the numbers that are read in
   */
  public static List<Long> readValues() {
    Scanner s = new Scanner(System.in);
    List<Long> l = new ArrayList<Long>();
    while (s.hasNextLong()) {
      l.add(s.nextLong());
    }
    return l;
  }

  /**
   * This is the main method. It parses the arguments and
   * kills the program if it detects invalid arguments
   * It then spins off the threads to do the calculation,
   * waiting for them to finish and finally outputs the 
   * calculated value
   * @param args The arguments that are passed in via the command line
   */
  public static void main(String[] args) {
    Integer fTarget = 0;
    Integer cores = 1;
    // If the user specified the number of cores parse it in (along with their fTarget)
    if (args.length == 2) {
      try {
        fTarget = Integer.parseInt(args[0]);
        cores = Integer.parseInt(args[1]);
      } catch (NumberFormatException e) {
        System.out.println("usage: java fcount <fTarget> [cores]");
        System.exit(1);
      }
      // If the user didn't specify a number of cores, use the default value of 1
    } else if (args.length == 1) {
      try {
        fTarget = Integer.parseInt(args[0]);
      } catch (NumberFormatException e) {
        System.out.println("usage: java fcount <fTarget> [cores]");
        System.exit(1);
      }
      // If they didn't even provide the right number of arguments, die.
    } else {
      System.out.println("usage: java fcount <fTarget> [cores]");
      System.exit(1);
    }

    // Make 10 threads and let them start running.
    List<Long> inputNums = readValues();
    int vCount = inputNums.size();
    Worker[] workers = new Worker[cores];
    for (int i = 0; i < cores; i++) {
      // This splits up the work as evenly as possible, using integer rounding. 
      workers[i] = new Worker(fTarget, inputNums.subList(i * vCount / cores, (i + 1) * vCount / cores));
      // Start the thread
      workers[i].start();
    }
    int total = 0;
    // Wait for each of the threads to terminate.
    try {
      for (int i = 0; i < workers.length; i++) {
        // Join with the thread and wait until it terminates
        workers[i].join();
        // Add the terminated thread's count to our total count.
        total += workers[i].count;
      }
    } catch (InterruptedException e) {
      System.out.println("Interrupted during join!");
    }
    System.out.println("Total: " + total);
  }
}

