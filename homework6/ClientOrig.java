import java.io.*;
import java.util.Scanner;
import java.net.Socket;
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import java.security.PrivateKey;
import java.security.KeyFactory;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.GeneralSecurityException;
import javax.xml.bind.DatatypeConverter;

/** Client supporting simple interactionw with the server. */
public class ClientOrig {
  public static void main( String[] args ) {
    // Complain if we don't get the right number of arguments. 
    if ( args.length != 1 ) {
      System.out.println( "Usage: Client <host>" );
      System.exit( -1 );
    }

    try {
      // Try to create a socket connection to the server.
      Socket sock = new Socket( args[ 0 ], Server.PORT_NUMBER );

      // Get formatted input/output streams for talking with the server.
      DataInputStream input = new DataInputStream( sock.getInputStream() );
      DataOutputStream output = new DataOutputStream( sock.getOutputStream() );

      // Get a username from the user and send it to the server.
      Scanner scanner = new Scanner( System.in );
      System.out.print( "Username: " );
      String name = scanner.nextLine();
      output.writeUTF( name );
      output.flush();

      // Try to read the user's private key.

      // Get the challenge string (really a byte array) from the server.
      byte[] challenge = ServerOriginal.getMessage( input );
      // Encrypt the challenge with our private key and send it back.
      // ...

      // Get the symmetric key from the server and make AES
      // encrypt/decrypt objects for it.
      // ...

      // Read commands from the user and print server responses.
      String request = "";
      System.out.print( "cmd> " );
      while ( scanner.hasNextLine() && ! ( request = scanner.nextLine() ).equals( "exit" ) ) {
        Server.putMessage( output, request.getBytes() );

        // Read and print the response.
        String response = new String( Server.getMessage( input ) );
        System.out.print( response );
        
        System.out.print( "cmd> " );
      }
      
      // Send the exit command to the server.
      Server.putMessage( output, request.getBytes() );
 
      // We are done communicating with the server.
      sock.close();
    } catch( IOException e ){
      System.err.println( "IO Error: " + e );
    }
  }
}