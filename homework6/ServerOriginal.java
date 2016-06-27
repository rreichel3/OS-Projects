import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;
import java.util.Random;
import java.util.ArrayList;
import java.util.Arrays;
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import java.security.PublicKey;
import java.security.KeyFactory;
import java.security.spec.X509EncodedKeySpec;
import java.security.GeneralSecurityException;
import javax.xml.bind.DatatypeConverter;

/** A server that keeps up with a public key for every user, along
    with a map with a location for every user. */
public class ServerOriginal {
  /** Port number used by the server */
  public static final int PORT_NUMBER = 26160;

  /** Record for an individual user. */
  private static class UserRec {
    // Name of this user.
    String name;

    // This user's public key.
    PublicKey publicKey;
    
    // Location for this user.
    int row = -1;
    int col = -1;
  }

  /** List of all the user records. */
  private ArrayList< UserRec > userList = new ArrayList< UserRec >();

  /** Current map, a 2D array of characters. */
  private char map[][];

  /** Read the map and all the users, done at program start-up. */
  private void readMap() throws Exception {
    Scanner input = new Scanner( new File( "map.txt" ) );

    // Read in the map.
    int height = input.nextInt();
    int width = input.nextInt();

    map = new char [ height ][];
    for ( int i = 0; i < height; i++ )
      map[ i ] = input.next().toCharArray();

    // Read in all the users.
    int userCount = input.nextInt();
    for ( int k = 0; k < userCount; k++ ) {
      // Create a record for the next user.
      UserRec rec = new UserRec();
      rec.name = input.next();
      
      // Get the key as a string of hex digits and turn it into a byte array.
      String hexKey = input.nextLine().trim();
      byte[] rawKey = DatatypeConverter.parseHexBinary( hexKey );
    
      // Make a key specification based on this key.
      X509EncodedKeySpec pubKeySpec = new X509EncodedKeySpec( rawKey );

      // Make an RSA key based on this specification
      KeyFactory keyFactory = KeyFactory.getInstance( "RSA" );
      rec.publicKey = keyFactory.generatePublic( pubKeySpec );

      // Make sure this user has a unique initial.
      for ( int i = 0; i < userList.size(); i++ )
        if ( rec.name.charAt( 0 ) == userList.get( i ).name.charAt( 0 ) )
          throw new Exception( "Duplicate user initials" );
      
      // Find this user on the map.
      for ( int i = 0; i < map.length; i++ )
        for ( int j = 0; j < map[ i ].length; j++ )
          if ( map[ i ][ j ] == rec.name.charAt( 0 ) ) {
            rec.row = i;
            rec.col = j;
          }
      
      if ( rec.row < 0 )
        throw new Exception( "User is not on the map" );

      // Add this user to the list of all users.
      userList.add( rec );
    }
  }

  /** Utility function to read a length then a byte array from the
      given stream.  TCP doesn't respect message boundaries, but this
      is essentially a technique for marking the start and end of
      each message in the byte stream.  This can also be used by the
      client. */
  public static byte[] getMessage( DataInputStream input ) throws IOException {
    int len = input.readInt();
    byte[] msg = new byte [ len ];
    input.readFully( msg );
    System.out.println("Read: " + msg + " which is " + msg.length);
    return msg;
  }

  /** Function analogous to the previous one, for sending messages. */
  public static void putMessage( DataOutputStream output, byte[] msg ) throws IOException {
    // Write the length of the given message, followed by its contents.
    output.writeInt( msg.length );
    output.write( msg, 0, msg.length );
    System.out.println("Writing: "+ msg + " which is " + msg.length);
    output.flush();

  }

  /** Function to handle interaction with a client.  Really, this should
      be run in a thread. */
  public void handleClient( Socket sock ) {
    try {
      // Get formatted input/output streams for this thread.  These can read and write
      // strings, arrays of bytes, ints, lots of things.
      DataOutputStream output = new DataOutputStream( sock.getOutputStream() );
      DataInputStream input = new DataInputStream( sock.getInputStream() );
      
      // Get the username.
      String username = input.readUTF();

      // Make a random sequence of bytes to use as a challenge string.
      Random rand = new Random();
      byte[] challenge = new byte [ 16 ];
      System.out.println(challenge);
      rand.nextBytes( challenge );
      System.out.println(challenge);
      putMessage(output, challenge);

      // Find this user.  We don't need to synchronize here, since the set of users never
      // changes.
      UserRec rec = null;
      for ( int i = 0; rec == null && i < userList.size(); i++ )
        if ( userList.get( i ).name.equals( username ) )
          rec = userList.get( i );
      // Did we find a record for this user?
      if ( rec != null ) {
        System.out.println("Challenge: " + challenge);
        // Send the client the challenge.
        putMessage( output, challenge );

        String request = new String( getMessage( input ) );

        // All request are single words, easy to dispatch based on the request.
        //while ( ! request.equals( "exit" ) ) {
          StringBuilder reply = new StringBuilder();
              
          // For now, just send back a copy of the request as the reply.
          reply.append( request + "\n" );  
          
          // Send the reply back to our client.
          putMessage( output, reply.toString().getBytes() );
              
          // Get the next command.
          request = new String( getMessage( input ) );
        //}
      }
    } catch ( IOException e ) {
      System.out.println( "IO Error: " + e );
    } finally {
      try {
        // Close the socket on the way out.
        sock.close();
      } catch ( Exception e ) {
      }
    }
  }

  /** Essentially, the main method for our server, as an instance method
      so we can access non-static fields. */
  public void run( String[] args ) {
    ServerSocket serverSocket = null;
    
    // One-time setup.
    try {
      // Read the map and the public keys for all the users.
      readMap();

      // Open a socket for listening.
      serverSocket = new ServerSocket( PORT_NUMBER );
    } catch( Exception e ){
      System.err.println( "Can't initialize server: " + e );
      System.exit( 1 );
    }
     
    // Keep trying to accept new connections and serve them.
    while( true ){
      try {
        // Try to get a new client connection.
        Socket sock = serverSocket.accept();

        // Handle interaction with this client.
        handleClient( sock );
      } catch( IOException e ){
        System.err.println( "Failure accepting client " + e );
      }
    }
  }

  public static void main( String[] args ) {
    // Make a server object, so we can have non-static fields.
    ServerOriginal server = new ServerOriginal();
    server.run( args );
  }
}