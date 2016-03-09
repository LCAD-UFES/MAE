import java.io.File;
import java.io.InputStream;
import java.net.Socket;
import java.net.ServerSocket;

public class FileReceiver implements Runnable {

	//TODO: Criar um arquivo de configuração para colocar as constantes abaixo
	//		e reservar uma porta para cada input do sensor associada ao filename da imagem.
	private static final int port = 4711;
	private static final String path = "../";

	private Socket socket;

	public static void main(String[] _) {
		try {
			ServerSocket listener = new ServerSocket(port);

			while (true) {
				FileReceiver file_rec = new FileReceiver();
				file_rec.socket = listener.accept();  

				new Thread(file_rec).start();
			}
		}
		catch (java.lang.Exception ex) {
			ex.printStackTrace(System.out);
		}
	}

	public void run() {
		try {
			InputStream in = socket.getInputStream();

			int nof_files = 1;//ByteStream.toInt(in);

			for (int cur_file=0;cur_file < nof_files; cur_file++) {
				String file_name = ByteStream.toString(in);

				File file=new File(path+file_name);

				ByteStream.toFile(in, file);
			}
		}
		catch (java.lang.Exception ex) {
			ex.printStackTrace(System.out);
		}
	}
} 
