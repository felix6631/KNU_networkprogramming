import java.io.*;
import org.json.JSONArray;
import org.json.JSONObject;

public class WriteJsonFileExample {
    public static void main(String args[]) {
        JSONObject obj = new JSONObject();

        obj.put("name", "Alice");
        obj.put("age",28);

        JSONObject address = new JSONObject();
        address.put("city","seoul");
        address.put("zip","12345");
        obj.put("address",address);

        JSONArray hobbies = new JSONArray();
        hobbies.put("reading");
        hobbies.put("swimming");

        obj.put("hobbies",hobbies);
        obj.put("spouse",false);

        String jsonString = obj.toString(4);
        System.out.println(jsonString);

        try(Writer file = new FileWriter("sample.json")) {
            file.write(jsonString);
            file.flush();
            file.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        System.out.println("sample.json saved");
    }
}
