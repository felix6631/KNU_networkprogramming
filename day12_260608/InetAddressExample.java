import java.net.InetAddress;
import java.net.UnknownHostException;

public class InetAddressExample {
    public static void main(String[] args) {
        try {
            InetAddress local = InetAddress.getLocalHost();
            System.out.println("local: "+local);
            System.out.println("Hostname: "+local.getHostName());
            System.out.println("My Computer IP Address: "+local.getHostAddress());

            InetAddress[] iaArray = InetAddress.getAllByName("www.google.com");
            for(InetAddress remote : iaArray) {
                System.out.println("Google IP Address: "+remote.getHostAddress());
            }
        } catch(UnknownHostException e) {
            e.printStackTrace();
        }
    }
}
