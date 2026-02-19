import grpc
import time
import random
import sensor_pb2
import sensor_pb2_grpc

def run():
    with grpc.insecure_channel('localhost:50051') as channel:
        stub = sensor_pb2_grpc.SensorServiceStub(channel)
        print("Simulator Aktif! Mengirim data sensor ke Docker...")

        while True:
            suhu = round(random.uniform(20.0, 35.0), 2)
            kelembaban = round(random.uniform(40.0, 60.0), 2)
            tekanan = round(random.uniform(1000.0, 1015.0), 2)

            request = sensor_pb2.SensorRequest(
                sensor_id=1,                       # int32
                sensor_name="DHT22-Sensor",         # string
                temperature=suhu,                  # double
                humidity=kelembaban,               # double
                pressure=tekanan,                  # double
                light_intensity=random.uniform(100, 500), # double
                timestamp=int(time.time() * 1000), # int64 (epoch ms)
                location="Kamar 123"             # string
            )

            try:
                response = stub.SendSensorData(request)
                
                if response.success:
                    print(f"-> [OK] Suhu: {suhu}°C | Pesan Server: {response.message}")
                else:
                    print(f"-> [GAGAL] Pesan Server: {response.message}")
                    
            except Exception as e:
                print(f"X Koneksi Error: {e}")

            time.sleep(2) 

if __name__ == '__main__':
    run()