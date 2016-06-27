import java.io.File;
import java.io.IOException;
import java.util.Scanner;
import java.util.InputMismatchException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.TreeSet;
import java.text.Collator;
import java.util.Iterator;
import java.util.List;
import java.util.LinkedList;
import java.net.DatagramSocket;
import java.net.DatagramPacket;


/** Server that responds to queries for listing objects and reporting
  * what user can access what object */
public class PermissionsServer {
  /** Port number used by the server.  */
  public static final int SERVER_PORT = 26160;
  
  /** Record for an object, with a name and a list of keys that can
    * access it. */
  static class ObjectRec {
    // Name for the object
    String name;

    // List of keys that can be used to access this object.
    ArrayList< Integer > keys = new   ArrayList< Integer >();
  }

  /** Record for an individual user. */
  static class UserRec {
    // Name of the user
    String name;

    // List of keys held by the user.
    ArrayList< Integer > keys = new ArrayList< Integer >();
  }

  // List of all known objects.
  private ArrayList< ObjectRec > objList = new ArrayList< ObjectRec >();

  // List of all known users.
  private ArrayList< UserRec > userList = new ArrayList< UserRec >();

  /** Read all the object information from a file named objects.txt and
      store it in objList. */
  private void readObjects() throws IOException {
    Scanner input = new Scanner( new File( "objects.txt" ) );
    while ( input.hasNext() ) {
      // Make a new object record and populate it.
      ObjectRec obj = new ObjectRec();
      obj.name = input.next();
      String line = input.nextLine();
      Scanner lineScanner = new Scanner( line );
      while ( lineScanner.hasNextInt() )
        obj.keys.add( lineScanner.nextInt() );
      objList.add( obj );
    }
  }

  /** Read all the user information from a file named users.txt and
      store it in userList. */
  private void readUsers() throws IOException {
    Scanner input = new Scanner( new File( "users.txt" ) );
    while ( input.hasNext() ) {
      // Make a new user record and populate it.
      UserRec rec = new UserRec();
      rec.name = input.next();
      String line = input.nextLine();
      Scanner lineScanner = new Scanner( line );
      while ( lineScanner.hasNextInt() )
        rec.keys.add( lineScanner.nextInt() );
      userList.add( rec );
    }
  }

  /** Process requests until the user kills us. */
  public void serveRequests() throws IOException {
    // You get to implement this function.
    // ...
    DatagramSocket sock = null;
    try {
      sock = new DatagramSocket( SERVER_PORT );
    } catch( IOException e ){
      System.err.println( "Can't create socket: " + e );
      System.exit( -1 );
    }
    
    // Reusable packet for receiving messages, hopefully big enough
    // for any message we'll receive.
    byte[] recvBuffer = new byte [ 1024 ];
    DatagramPacket recvPacket = new DatagramPacket( recvBuffer, recvBuffer.length );

    // Keep reading messages and sending responses.
      while( true ){
        // Get a packet.
        try {
          sock.receive( recvPacket );

          // Turn it into a string.
          String str = new String( recvBuffer, 0, recvPacket.getLength() );

          // Let the user know we got something
          //System.out.printf( "Got: %s\n", str );
        
          // Change it, just to show we did something.
          Scanner input = new Scanner(str);
          // Get the next thing on input
          String command = input.next();
          // initialize a return string
          String returnStr = "";
          // If our command is the list command
          if (command.equals("list")) {
            // Initialize a collection of in a TreeSet (To keep stuff sorted)
            Collection<String> returnObjects = new TreeSet<String>(Collator.getInstance());
            // Create an iterator to iterate over the objects
            Iterator<ObjectRec> objects = objList.iterator();
            // Iterate.
            while (objects.hasNext())
              // Add the object to our sorted colletion
              returnObjects.add(objects.next().name);
            // Create another iterator on the collecion
            Iterator<String> objIterator = returnObjects.iterator();
            // Iterate.
            while (objIterator.hasNext())
              // Create the return string
              returnStr += objIterator.next() + "\n";
          }
          // Else if we received an object command
          else if (command.equals("obj")) {
            // Initialize a new collection that uses a tree set to stay sorted
            Collection<String> validUsers = new TreeSet<String>(Collator.getInstance());
            // Get the object name from input
            String objectName = input.next();
            // Create a listttt
            List<Integer> objectKeys = null;
            // For each item in the object list see if it matches our requested object (Brute force search)
            for (int i = 0; i < objList.size(); i++) {
              // If it matches, grab its key list AND break from the loop
              if(objList.get(i).name.equals(objectName)) {
                objectKeys = objList.get(i).keys;
                break;
              }
            }
            // Create an iterator to go over the users
            Iterator<UserRec> users = userList.iterator();
            // For each user...
            while (users.hasNext()) {
              // Store current user
              UserRec temp = users.next();
              // Grab their keys
              Iterator<Integer> userKeys = temp.keys.iterator();
              // Go through all their keys seeing if any match the ones associated with our object
              while(userKeys.hasNext())
                if (objectKeys.contains(userKeys.next()))
                  validUsers.add(temp.name);
            }
            // ANOTHER ITERATOR. YASSS ITERATORS ARE BAE
            Iterator<String> userNames = validUsers.iterator();
            // This time we're again creating the response string
            while (userNames.hasNext()) {
              returnStr += userNames.next() + "\n";
            }
          }
          // If the command didn't work right, throw an exception to be caught farther down
          else 
            throw new Exception("Something happened");

          
          // Turn the string into a datagram packet, and send it back where the
          // messagee came from.
          byte[] sendBuffer = returnStr.getBytes();
          DatagramPacket sendPacket = new DatagramPacket( sendBuffer, sendBuffer.length,
                                                          recvPacket.getAddress(),
                                                          recvPacket.getPort() );
          sock.send( sendPacket );
        } catch( Exception e) {

          System.out.println("Bad request" + e);
          e.printStackTrace();
          continue;
        }
      }
  }

  /** Make an instance of this object, get it to read its configuration then serve
      requests over UDP. */
  public static void main( String[] args ) throws IOException {
    PermissionsServer server = new PermissionsServer();
    server.readObjects();
    server.readUsers();
    server.serveRequests();
  }
}
