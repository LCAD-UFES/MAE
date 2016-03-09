import java.io.OutputStream;
import java.io.File;
import java.net.Socket;


public class FileSender {

	//TODO: Criar um arquivo de configuração para colocar as constantes abaixo
	//		e reservar uma porta para cada input do sensor e nome do arquivo de imagem.
	private static final int    port = 4711;
	private static final String host = "localhost";

	public static void main(String[] args) {
		try {
			Socket       socket = new Socket(host, port);
			OutputStream os     = socket.getOutputStream();

			//int cnt_files = args.length;
			int cnt_files = 1;

			// How many files?
			//ByteStream.toStream(os, cnt_files);

			for (int cur_file=0; cur_file<cnt_files; cur_file++) {
				//String filename = args[cur_file];
				String filename = "nc_sensors_input1.ppm";
				ByteStream.toStream(os, filename);
				ByteStream.toStream(os, new File(filename));
			}
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
	}
} 
