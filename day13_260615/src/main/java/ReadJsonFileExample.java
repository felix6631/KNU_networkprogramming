import java.io.*;
import org.json.JSONArray;
import org.json.JSONObject;

public class ReadJsonFileExample {
    public static void main(String args[]) {
        try(Reader reader = new FileReader("sample.json")) {
            char[] buffer = new char[1024];
            int len = reader.read(buffer);
            String jsonReadString = new String(buffer, 0, len);

            JSONObject obj = new JSONObject(jsonReadString);
            System.out.println(obj.toString(4));
            System.out.println("name" + obj.getString("name"));
            System.out.println("city: " + obj.getJSONObject("address").getString("city"));

            JSONArray hobbies = obj.getJSONArray("hobbies");
            System.out.println("hobbies: " + hobbies);

            for(int i=0; i<hobbies.length(); i++) {
                System.out.println(hobbies.get(i));
            }
            reader.close();
        } catch(IOException e) {
            e.printStackTrace();
        }   
    }
}
