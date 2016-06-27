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
public class Server {
  /** Port number used by the server */
  public static final int PORT_NUMBER = 26160;
  private static Object lock = new Object();

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
  private static ArrayList< UserRec > userList = new ArrayList< UserRec >();

  /** Current map, a 2D array of characters. */
  private static char map[][];

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
  public static class ClientConnHandle implements Runnable {
    private Socket sock;
    DataOutputStream output;
    DataInputStream input;
    public ClientConnHandle(Socket sock) {
      this.sock = sock;
      try {
        this.output = new DataOutputStream( sock.getOutputStream() );
        this.input = new DataInputStream( sock.getInputStream() );
      } catch (IOException e) {

      }
    }
    private String moveUp(UserRec rec) {
      if (rec.row != 0 && (map[rec.row-1][rec.col] == '.') ) {
        // Handle stuff
        map[rec.row][rec.col] = '.';
        map[rec.row-1][rec.col] = rec.name.charAt( 0 );
        return "OK\n";
      }
      else if (map[rec.row-1][rec.col] == '*' && rec.row-1 != 0 && map[rec.row-2][rec.col] == '*') {
        // Handle boulder
        map[rec.row][rec.col] = '.';
        map[rec.row-1][rec.col] = rec.name.charAt( 0 );
        map[rec.row-2][rec.col] = '*';
        return "OK\n";
      }
      else 
        return "Blocked\n";
    }

    private String moveDown(UserRec rec) {
      if (rec.row != map.length-1 && (map[rec.row+1][rec.col] == '.')) {
        // Handle stuff
        map[rec.row][rec.col] = '.';
        map[rec.row+1][rec.col] = rec.name.charAt( 0 );
        return "OK\n";
      }
      else if (map[rec.row+1][rec.col] == '*' && rec.row+2 < map.length && map[rec.row+2][rec.col] == '*') {
        // Handle boulder
        map[rec.row][rec.col] = '.';
        map[rec.row+1][rec.col] = rec.name.charAt( 0 );
        map[rec.row+2][rec.col] = '*';
        return "OK\n";
      }
      else 
        return "Blocked\n";
    }

    private String moveLeft(UserRec rec) {
      if (rec.col != 0 && (map[rec.row][rec.col-1] == '.') ) {
        // Handle stuff
        map[rec.row][rec.col] = '.';
        map[rec.row][rec.col-1] = rec.name.charAt( 0 );
        return "OK\n";
      }
      else if (map[rec.row][rec.col-1] == '*' && rec.col-2 >= 0 && map[rec.row][rec.col-2] == '*') {
        // Handle boulder
        map[rec.row][rec.col] = '.';
        map[rec.row][rec.col-1] = rec.name.charAt( 0 );
        map[rec.row][rec.col-2] = '*';
        return "OK\n";
      }
      else 
        return "Blocked\n";
    }

    private String moveRight(UserRec rec) {
      if (rec.col+1 < map[0].length && (map[rec.row][rec.col+1] == '.') ) {
        // Handle stuff
        map[rec.row][rec.col] = '.';
        map[rec.row][rec.col+1] = rec.name.charAt( 0 );
        return "OK\n";
      }
      else if (map[rec.row][rec.col+1] == '*' && rec.col+2 < map.length && map[rec.row][rec.col+2] == '*') {
        // Handle boulder
        map[rec.row][rec.col] = '.';
        map[rec.row][rec.col+1] = rec.name.charAt( 0 );
        map[rec.row][rec.col+2] = '*';
        return "OK\n";
      }
      else 
        return "Blocked\n";
    }
    public void run() {
      try {
        // Get the user name.
        String username = input.readUTF();

        // Make a random sequence of bytes to use as a challenge string.
        Random rand = new Random();
        byte[] challenge = new byte [ 16 ];
        rand.nextBytes( challenge );

        // Make a session key for communicating over AES.  We use it later, if the
        // client successfully authenticates.
        byte[] sessionKey = new byte [ 16 ];
        rand.nextBytes( sessionKey );

        // Find this user.  We don't need to synchronize here, since the set of users never
        // changes.
        UserRec rec = null;
        for ( int i = 0; rec == null && i < userList.size(); i++ )
          if ( userList.get( i ).name.equals( username ) )
            rec = userList.get( i );

        // Did we find a record for this user?
        if ( rec != null ) {
          // We need this to make sure the client properly encrypted
          // the challenge.
          Cipher RSADecrypter = Cipher.getInstance( "RSA" );
          RSADecrypter.init( Cipher.DECRYPT_MODE, rec.publicKey );

          // And this to send the session key 
          Cipher RSAEncrypter = Cipher.getInstance( "RSA" );
          RSAEncrypter.init( Cipher.ENCRYPT_MODE, rec.publicKey );
          //System.out.println("before sending challenge string");

          // Send the client the challenge.
          putMessage( output, challenge );
            
          // Get back the client's encrypted challenge.
          // ...
          byte[] challengeResp  = getMessage(input);
          System.out.println("Response : " + challengeResp);
          // Make sure the client properly encrypted the challenge.
          // ...
          byte[] decResp = RSADecrypter.doFinal(challengeResp);
          System.out.println("DResponse: " + decResp);
          
          

          if(! decResp.equals(challenge)) {
            sock.close();
            return;
          }
          //////////////////////////////////////////////////////////////////////////////////////
          // Send the client the session key (encrypted)
          // ...
          putMessage(output, RSAEncrypter.doFinal(sessionKey));
          // Make AES cipher objects to encrypt and decrypt with
          // the session key.
          // ...
          SecretKey aesKey = new SecretKeySpec( sessionKey, "AES" );
          Cipher AESEncrypter = Cipher.getInstance("AES/ECB/PKCS5Padding");
          AESEncrypter.init(Cipher.ENCRYPT_MODE, aesKey);

          Cipher AESDecrypter = Cipher.getInstance("AES/ECB/PKCS5Padding");
          AESDecrypter.init(Cipher.DECRYPT_MODE, aesKey);
          // Get the first client request

          String request = new String( AESDecrypter.doFinal(getMessage( input )) );

          // All request are single words, easy to dispatch based on the request.
          while ( ! request.equals( "exit" ) ) {
            StringBuilder reply = new StringBuilder();
            synchronized(lock) {
              if (request.equals("map")) {
                // Handle map command
                for(int i = 0; i < map.length; i++) {
                  for(int j = 0; j < map[i].length; j++) {
                    reply.append(map[i][j]);
                  }
                  reply.append("\n");
                }
              }
              else if (request.equals("up")) {
                reply.append(moveUp(rec));
              }
              else if (request.equals("down")) {
                reply.append(moveDown(rec));
              }
              else if (request.equals("left")) {
                reply.append(moveLeft(rec));
              }
              else if (request.equals("right")) {
                reply.append(moveRight(rec));
              }
            }
            // Send the reply back to our client.

            putMessage( output, AESEncrypter.doFinal(reply.toString().getBytes()) );
                
            // Get the next command.
            request = new String( AESDecrypter.doFinal(getMessage( input ) ) );
          }
        }
      } catch ( IOException e ) {
        System.out.println( "IO Error: " + e );
      } catch( GeneralSecurityException e ){
        System.err.println( "Encryption error: " + e );
      } finally {
        try {
          // Close the socket on the way out.
          sock.close();
        } catch ( Exception e ) {
        }
      }
    }
  }
  public void handle(Socket socke) {
      Socket sock = socke;
      try {
        
      
        DataOutputStream output = new DataOutputStream( sock.getOutputStream() );
        DataInputStream input = new DataInputStream( sock.getInputStream() );
        // Get the user name.
        String username = input.readUTF();

        // Make a random sequence of bytes to use as a challenge string.
        Random rand = new Random();
        byte[] challenge = new byte [ 16 ];
        rand.nextBytes( challenge );

        // Make a session key for communicating over AES.  We use it later, if the
        // client successfully authenticates.
        byte[] sessionKey = new byte [ 16 ];
        rand.nextBytes( sessionKey );

        // Find this user.  We don't need to synchronize here, since the set of users never
        // changes.
        UserRec rec = null;
        for ( int i = 0; rec == null && i < userList.size(); i++ )
          if ( userList.get( i ).name.equals( username ) )
            rec = userList.get( i );

        // Did we find a record for this user?
        if ( rec != null ) {
          // We need this to make sure the client properly encrypted
          // the challenge.
          Cipher RSADecrypter = Cipher.getInstance( "RSA" );
          RSADecrypter.init( Cipher.DECRYPT_MODE, rec.publicKey );

          // And this to send the session key 
          Cipher RSAEncrypter = Cipher.getInstance( "RSA" );
          RSAEncrypter.init( Cipher.ENCRYPT_MODE, rec.publicKey );
          //System.out.println("before sending challenge string");

          // Send the client the challenge.
          putMessage( output, challenge );
            
          // Get back the client's encrypted challenge.
          // ...
          byte[] challengeResp  = getMessage(input);
          System.out.println("Response : " + challengeResp);
          // Make sure the client properly encrypted the challenge.
          // ...
          byte[] decResp = RSADecrypter.doFinal(challengeResp);
          System.out.println("DResponse: " + decResp);
          
          

          if(! decResp.equals(challenge)) {
            sock.close();
            return;
          }
          //////////////////////////////////////////////////////////////////////////////////////
          // Send the client the session key (encrypted)
          // ...
          putMessage(output, RSAEncrypter.doFinal(sessionKey));
          // Make AES cipher objects to encrypt and decrypt with
          // the session key.
          // ...
          SecretKey aesKey = new SecretKeySpec( sessionKey, "AES" );
          Cipher AESEncrypter = Cipher.getInstance("AES/ECB/PKCS5Padding");
          AESEncrypter.init(Cipher.ENCRYPT_MODE, aesKey);

          Cipher AESDecrypter = Cipher.getInstance("AES/ECB/PKCS5Padding");
          AESDecrypter.init(Cipher.DECRYPT_MODE, aesKey);
          // Get the first client request

          String request = new String( AESDecrypter.doFinal(getMessage( input )) );

          // All request are single words, easy to dispatch based on the request.
          while ( ! request.equals( "exit" ) ) {
            StringBuilder reply = new StringBuilder();
            synchronized(lock) {
              if (request.equals("map")) {
                // Handle map command
                for(int i = 0; i < map.length; i++) {
                  for(int j = 0; j < map[i].length; j++) {
                    reply.append(map[i][j]);
                  }
                  reply.append("\n");
                }
              }
              else if (request.equals("up")) {
                reply.append(moveUp(rec));
              }
              else if (request.equals("down")) {
                reply.append(moveDown(rec));
              }
              else if (request.equals("left")) {
                reply.append(moveLeft(rec));
              }
              else if (request.equals("right")) {
                reply.append(moveRight(rec));
              }
            }
            // Send the reply back to our client.

            putMessage( output, AESEncrypter.doFinal(reply.toString().getBytes()) );
                
            // Get the next command.
            request = new String( AESDecrypter.doFinal(getMessage( input ) ) );
          }
        }
      } catch ( IOException e ) {
        System.out.println( "IO Error: " + e );
      } catch( GeneralSecurityException e ){
        System.err.println( "Encryption error: " + e );
      } finally {
        try {
          // Close the socket on the way out.
          sock.close();
        } catch ( Exception e ) {
        }
      }
    }
    private String moveUp(UserRec rec) {
      if (rec.row != 0 && (map[rec.row-1][rec.col] == '.') ) {
        // Handle stuff
        map[rec.row][rec.col] = '.';
        map[rec.row-1][rec.col] = rec.name.charAt( 0 );
        return "OK\n";
      }
      else if (map[rec.row-1][rec.col] == '*' && rec.row-1 != 0 && map[rec.row-2][rec.col] == '*') {
        // Handle boulder
        map[rec.row][rec.col] = '.';
        map[rec.row-1][rec.col] = rec.name.charAt( 0 );
        map[rec.row-2][rec.col] = '*';
        return "OK\n";
      }
      else 
        return "Blocked\n";
    }

    private String moveDown(UserRec rec) {
      if (rec.row != map.length-1 && (map[rec.row+1][rec.col] == '.')) {
        // Handle stuff
        map[rec.row][rec.col] = '.';
        map[rec.row+1][rec.col] = rec.name.charAt( 0 );
        return "OK\n";
      }
      else if (map[rec.row+1][rec.col] == '*' && rec.row+2 < map.length && map[rec.row+2][rec.col] == '*') {
        // Handle boulder
        map[rec.row][rec.col] = '.';
        map[rec.row+1][rec.col] = rec.name.charAt( 0 );
        map[rec.row+2][rec.col] = '*';
        return "OK\n";
      }
      else 
        return "Blocked\n";
    }

    private String moveLeft(UserRec rec) {
      if (rec.col != 0 && (map[rec.row][rec.col-1] == '.') ) {
        // Handle stuff
        map[rec.row][rec.col] = '.';
        map[rec.row][rec.col-1] = rec.name.charAt( 0 );
        return "OK\n";
      }
      else if (map[rec.row][rec.col-1] == '*' && rec.col-2 >= 0 && map[rec.row][rec.col-2] == '*') {
        // Handle boulder
        map[rec.row][rec.col] = '.';
        map[rec.row][rec.col-1] = rec.name.charAt( 0 );
        map[rec.row][rec.col-2] = '*';
        return "OK\n";
      }
      else 
        return "Blocked\n";
    }

    private String moveRight(UserRec rec) {
      if (rec.col+1 < map[0].length && (map[rec.row][rec.col+1] == '.') ) {
        // Handle stuff
        map[rec.row][rec.col] = '.';
        map[rec.row][rec.col+1] = rec.name.charAt( 0 );
        return "OK\n";
      }
      else if (map[rec.row][rec.col+1] == '*' && rec.col+2 < map.length && map[rec.row][rec.col+2] == '*') {
        // Handle boulder
        map[rec.row][rec.col] = '.';
        map[rec.row][rec.col+1] = rec.name.charAt( 0 );
        map[rec.row][rec.col+2] = '*';
        return "OK\n";
      }
      else 
        return "Blocked\n";
    }


  /** Essentially, the main method for our server, as an instance method
      so we can access non-static fields. */
  private void run( String[] args ) {
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
        handle(sock);
      } catch( IOException e ){
        System.err.println( "Failure accepting client " + e );
      }
    }
  }

  public static void main( String[] args ) {
    // Make a server object, so we can have non-static fields.
    Server server = new Server();
    server.run( args );
  }
}
