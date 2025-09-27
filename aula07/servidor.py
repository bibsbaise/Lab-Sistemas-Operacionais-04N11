import grpc
from concurrent import futures
import time
import random
import string

import urls_pb2
import urls_pb2_grpc

# Implementação do serviço EncurtadorURL
class EncurtadorURLService(urls_pb2_grpc.EncurtadorURLServicer):
    def __init__(self):
        # Dicionário em memória para armazenar os códigos e URLs
        self.url_map = {}

    # Método para encurtar URL
    def EncurtarURL(self, request, context):
        url_longa = request.url_longa

        # Gera um código curto aleatório de 6 caracteres
        codigo_curto = ''.join(random.choices(string.ascii_letters + string.digits, k=20))

        # Armazena a associação codigo_curto -> url_longa
        self.url_map[codigo_curto] = url_longa

        # Monta a URL curta (simulando um domínio local)
        url_curta = f"http://localhost:50051/{codigo_curto}"

        return urls_pb2.RespostaEncurtar(url_curta=url_curta)

    # Método para obter a URL longa
    def ObterURLLonga(self, request, context):
        codigo_curto = request.codigo_curto

        # Busca a URL longa no dicionário
        url_longa = self.url_map.get(codigo_curto, "")

        return urls_pb2.RespostaObter(url_longa=url_longa)


# Função para iniciar o servidor gRPC
def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    urls_pb2_grpc.add_EncurtadorURLServicer_to_server(EncurtadorURLService(), server)

    # Porta padrão 50051
    server.add_insecure_port('[::]:50051')
    server.start()
    print("Servidor gRPC rodando na porta 50051...")
    try:
        while True:
            time.sleep(86400)  # Mantém o servidor ativo
    except KeyboardInterrupt:
        server.stop(0)


if __name__ == "__main__":
    serve()
