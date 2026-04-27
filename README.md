# Testing the ELZ v1.0 Algorithm
## in Contiki-NG / COOJA

### Step 1: Install Docker
sudo apt update
sudo apt install docker.io docker-compose-v2
sudo usermod -aG docker $USER

### Step 2: Clone Contiki-NG
git clone https://github.com/contiki-ng/contiki-ng.git
cd contiki-ng
git submodule update --init --recursive

### Step 3: Pull the Docker image
docker pull contiker/contiki-ng

### Step 4: Launch COOJA
docker run --privileged \
  --net=host \
  -e DISPLAY=$DISPLAY \
  -v $HOME/.Xauthority:/home/user/.Xauthority \
  -v $(pwd):/home/user/contiki-ng \
  -ti contiker/contiki-ng bash
# Inside the container:
cd tools/cooja
./gradlew run   

### Step 5: Verify with a test simulation
In COOJA GUI: File -> Open Simulation -> Browse to:
  examples/elz-contiki-app/simulations/scenario-a-star.csc
Click Start — you should see nodes exchanging packets.
### 1.2 Directory Structure for elz Project
#### elz-contiki-app/
    elz/                 
      Makefile
      project-conf.h
      elz-coordinator.c
      elz-lcn-node.c
      elz-nn-node.c 
      elz-csma-config.c
      elz-csma-config.h
      elz-atpc.c
      elz-atpc.h
      elz-queue.c
      elz-queue.h
      elz-metrics.c
      elz-metrics.h
      simulations/
        scenario-a-star.csc    <-- COOJA simulation files
        scenario-b-cluster.csc
        scenario-c-dense.csc
      scripts/
        parse-logs.py
        plot-results.py
        run-batch.py

###Contiki-NG Stack Patches
Several ELZ features require minor modifications to the Contiki-NG source tree. 
Patch: Runtime-Adjustable CSMA Parameters
By default, macMinBE and macMaxBE are compile-time constants. This patch makes them runtime-adjustable variables.

#### File: os/net/mac/csma/csma.h
Add the following extern declarations:
/* --- ELZ PATCH: runtime CSMA parameters --- */
extern uint8_t csma_min_be;
extern uint8_t csma_max_be;
extern uint8_t csma_max_backoff;
/* --- END PATCH --- */

#### File: os/net/mac/csma/csma-output.c
/* IN send_one_packet(): */
/*ADD  BEFORE the call to NETSTACK_RADIO.prepare(): */
 
/* --- ELZ PATCH: per-destination TX power --- */
#ifdef ELZ_ATPC_ENABLED
{
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  int8_t pwr = elz_atpc_get_power(dest);
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, pwr);
}
#endif
/* --- END PATCH --- */

#Building the Firmware

make TARGET=cooja clean
make TARGET=cooja elz-coordinator
make TARGET=cooja elz-lcn-node DEFINES=NODE_IS_LCN=1
make TARGET=cooja elz-nn-node


