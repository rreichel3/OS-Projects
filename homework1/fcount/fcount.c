import java.io.*;
import java.util.*;

/**
 * Counts the number of factors of a specified number within a certain range.
 * 
 * @author Robert Reichel
 * @file Fcount.java
 */

public class Fcount {

    private static class Worker extends Thread {
        List<Long> numbers;
        int fTarget;
        public int count = 0;

        public Worker(int fTarget, List<Long> stuffToUse) {
            this.fTarget = fTarget;
            this.numbers = stuffToUse;
        }

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
    
    public static List<Long> readValues() {
        Scanner s = new Scanner(System.in);
        List<Long> l = new ArrayList<Long>();
        while (s.hasNextLong()) {
            l.add(s.nextLong());
        }
        return l;
    }

    /** Make a thread and wait for it to do something. */
    public static void main(String[] args) {
        Integer fTarget = 0;
        Integer cores = 1;
        if (args.length == 3) {
            try {
                fTarget = Integer.parseInt(args[1]);
                cores = Integer.parseInt(args[2]);
            } catch (NumberFormatException e) {
                System.out.println("usage: java fcount <fTarget> [cores]");
            }
        } else if (args.length == 2) {
            try {
                fTarget = Integer.parseInt(args[1]);
            } catch (NumberFormatException e) {
                System.out.println("usage: java fcount <fTarget> [cores]");
            }
        } else
            System.out.println("usage: java fcount <fTarget> [cores]");

        // Make 10 threads and let them start running.
        List<Long> inputNums = readValues();
        int vCount = inputNums.size();
        Worker[] workers = new Worker[cores];
        for (int i = 0; i < workers.length; i++) {
            workers[i] = new Worker(fTarget, inputNums.subList(vCount/ cores * i, vCount / cores * (i + 1)));
            workers[i].start();
        }
        int total = 0;
        // Wait for each of the threads to terminate.
        try {
            for (int i = 0; i < workers.length; i++) {
                workers[i].join();
                total += workers[i].count;
            }
        } catch (InterruptedException e) {
            System.out.println("Interrupted during join!");
        }
        System.out.println("Total: " + total);
    }
}

