import java.io.IOException;
import java.net.DatagramSocket;
import java.net.DatagramPacket;
import java.net.SocketTimeoutException;
import java.net.InetAddress;

/** Client program to request permissions information from the server,
    either a list of objects or the set of users who can access a particular
    object. */
public class PermissionsClient {
  /** Port number at which the client receives packets.  Has to be different
      from the server's port, in case we run on the same host. Be sure
      to use your own, randomly assigned port number. */
  public static final int SERVER_PORT = 26160;
  public static final int CLIENT_PORT = 26161;
  public static void main( String[] args ) {
    // Make sure the command-line arguments are good.
    if ( args.length != 2 ) {
      System.err.println( "usage: PermissionsClient <server_host> <object>" );
      System.err.println( "       PermissionsClient <server_host> list" );
      System.exit( 0 );
    }
 
    // Packet for receiving messages, hopefully big enough for any message we get.
    byte[] recvBuffer = new byte [ 1024 ];
    String str = "";
    DatagramPacket recvPacket = new DatagramPacket( recvBuffer, recvBuffer.length );
    // If the command is just list, then send it on
    if (args[1].equals("list"))
      str = "list";
    // Otherwise, construct a response string to match our UDP communication protocol
    else
      str = "obj " + args[1];

    try {
      // Make a socket for sending and receiving messages.
      DatagramSocket sock = new DatagramSocket( CLIENT_PORT );
      sock.setSoTimeout(2000);
      // Get the server's address just once.
      InetAddress[] addrList = InetAddress.getAllByName( args[ 0 ] );

      // Construct a packet containing the user's message.
      byte[] buffer = str.getBytes();
      DatagramPacket sendPacket = new DatagramPacket( buffer, buffer.length,
                                                      addrList[ 0 ],
                                                      SERVER_PORT );
      sock.send( sendPacket );
          
      // Get a response from the server.
      sock.receive( recvPacket );
          
      // Turn the response into a string, and print it.
      String response = new String( recvBuffer, 0, recvPacket.getLength() );
      System.out.print( response );

    } catch(SocketTimeoutException e) {
      System.err.println( "Message lost" );
    } catch( IOException e ){
      System.err.println( "Error in communicating with the server" + e );
    } 
  }
}
