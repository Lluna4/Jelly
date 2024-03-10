defmodule TCPServer do
  def start(port) do
    {:ok, sock} = :gen_tcp.listen(port, [:binary, active: false])
    IO.puts("Listening on port #{port}")
    accept_loop(sock)
  end

  def accept_loop(sock) do
    {:ok, socket} = :gen_tcp.accept(sock, :infinity)
    spawn(fn -> handle_client(socket) end)
    accept_loop(sock)
  end

  defp handle_client(sock) do
    case :gen_tcp.recv(sock, 0) do
      {:ok, data} ->
        parse_double(data, sock)
    end
  end

  defp parse_double(data, sock) do
    <<dbl::float-size(64), rest::binary>> = data
    {dbl, rest}
    :gen_tcp.send(sock, dbl)
  end

end

TCPServer.start(8080)
