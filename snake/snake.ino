const int TX_CLK = A5;
const int TX_DATA = A4;
const int DISP_DISABLE = A3;
const int RX_DATA = A2;
const int SPEAKER = A1;

constexpr
int dim [2] = {5, 3};
const int inp = 2;
const int tx_del = 0;
const int on_time = 1;

bool grid [dim[0]][dim[1]];
bool keys [inp] = {false, false};
int wait = 0;
int event [inp] = {-1, -1};
bool speaker = false;
int sound = 0;

bool get_pixel(int x, int y){
  return grid[x][y];
}

void set_pixel(int x, int y, bool val){
  grid[x][y] = val;
}

void clr(){
  for(int y = 0; y < dim[1]; y ++){
    for (int x = 0; x < dim[0]; x ++){
      set_pixel(x, y, false);
    }
  }
}

void fill(){
  for(int y = 0; y < dim[1]; y ++){
    for (int x = 0; x < dim[0]; x ++){
      set_pixel(x, y, true);
    }
  }
}

void buzz(int cycles){
  sound = cycles;
}

void transmit(bool val){
  digitalWrite(TX_DATA, val);
  delay(tx_del);
  digitalWrite(TX_CLK, HIGH);
  delay(tx_del);
  digitalWrite(TX_CLK, LOW);
  digitalWrite(TX_DATA, LOW);
}

void refresh(){
  for (int y = 0; y < dim[1] + inp; y ++){
    for (int i = 2; i >= 0; i --){
      transmit(i != y);
    }
    if (y < dim[1]){
      for (int x = dim[0] - 1; x >= 0; x --){
        transmit(get_pixel(x, y));
      }
      digitalWrite(DISP_DISABLE, LOW);
      delay(on_time);
      digitalWrite(DISP_DISABLE, HIGH);
    }
    
    else{
      for (int i = dim[0] - 1; i >= 0; i --){
        transmit(i == y - dim[1]);
      }
      digitalWrite(DISP_DISABLE, LOW);
      delay(on_time);
      bool key = digitalRead(RX_DATA);
      if (keys[y - dim[1]] != key){
        event[y - dim[1]] = (int) key;
        keys[y - dim[1]] = key;
      }
      digitalWrite(DISP_DISABLE, HIGH);
    }
  }
  if (sound){
    digitalWrite(SPEAKER, speaker);
    speaker = !speaker;
    sound --;
  }
  else{
    digitalWrite(SPEAKER, LOW);
  }
}

int get_event(int key){
  int copy = event[key];
  event[key] = -1;
  return copy;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TX_CLK, OUTPUT);
  pinMode(TX_DATA, OUTPUT);
  pinMode(DISP_DISABLE, OUTPUT);
  pinMode(SPEAKER, OUTPUT);
  pinMode(RX_DATA, INPUT);
  
  digitalWrite(DISP_DISABLE, HIGH);

  pinMode(A0, INPUT);
  randomSeed(analogRead(A0));
  
  app_init();
}

void loop() {
  if (!wait){
    wait = app();
  }
  else{
    wait --;
  }
  refresh();
}

//APP CODE

int pos [15][2];
int dir [2];
int food [2];
int len;
int snake_step;
int snake_stage;
int food_step;
int food_stage;
bool food_show;
bool dead;
int stage;
int ptr;
int death_slp;
int start_slp;
float delta_step;
int def_food_step;
int fast_food_step;
bool crash_flash;
int crash_timer;
int crash_step;
int flash_step;

void draw(){
  clr();
  set_pixel(food[0], food[1], food_show);
  for (int i = 0; i < len; i ++){
    set_pixel(pos[i][0], pos[i][1], true);
  }
}

int ani_play(int* data, int data_size, int anim_del){
  for (int i = 0; i < 15; i ++){
    bool col = data[ptr] & (0x8000 >> i);
    int x = i % 5;
    int y = i / 5;
    set_pixel(x, y, col);
  }
  buzz(2);
  
  ptr ++;
  if (ptr == data_size){
    return -1;
  }
  else{
    return anim_del;
  }
}

const int start_anim_del = 20;
const int start_anim_len = 10;
const int start_anim [10] = {
  0b0000000000000000,
  0b1010100000010100,
  0b1010111111010100,
  0b1111111111111110,
  0b1111111111111110,
  0b1111111111111110,
  0b1111111111111110,
  0b0101011111101010,
  0b0101000000101010,
  0b0000000000000000
};

const int end_anim_del = 7;
const int end_anim_len = 27;
const int end_anim [27] = {
  0b0000000000000000,
  0b0000100001000010,
  0b0000100001000010,
  0b0001100011000110,
  0b0001100011000110,
  0b0011100111001110,
  0b0011100111001110,
  0b0111101111011110,
  0b0111101111011110,
  0b1111111111111110,
  0b0000000000000000,
  
  0b0100000000000000,
  0b0100000000000000,
  
  0b0100000100000000,
  0b0100000100000000,
  
  0b0100000100000100,
  0b0100000100000100,
  0b0100000100000100,
  0b0100000100000100,
  0b0100000100000100,
  0b0100000100000100,
  
  0b0101000100000100,
  0b0101000100000100,
  0b0101000100000100,
  0b0101000100000100,
  
  0b0101000100010100,
  0b0101000100010100
};

void app_init(){
  pos[0][0] = -1;
  pos[0][1] = 1;
  dir[0] = 1;
  dir[1] = 0;
  len = 2;
  snake_step = 100;
  snake_stage = 100;
  food_show = false;
  food[0] = random(2, 5);
  food[1] = random(0, 3);
  dead = false;
  ptr = 0;
  stage = 0;
  death_slp = 400;
  start_slp = 250;
  def_food_step = 50;
  fast_food_step = 10;
  food_step = def_food_step;
  food_stage = 25;
  delta_step = 0.95;
  crash_flash = true;
  crash_step = 10;
  crash_timer = 0;
  flash_step = 10;
}

int app(){
  if (stage == 0){
    int del = ani_play(start_anim, start_anim_len, start_anim_del);
    if (del < 0){
      stage = 1;
      ptr = 0;
      return start_slp;
    }
    else{
      return del;
    }
  }
  
  else if (stage == 1){
    if (snake_stage == snake_step){
      for (int i = len - 1; i >= 0; i --){ //shift snake tail
        pos[i + 1][0] = pos[i][0];
        pos[i + 1][1] = pos[i][1];
      }
      
      pos[0][0] += dir[0]; //update head position
      pos[0][1] += dir[1];
      if (pos[0][0] == dim[0]){ //readjust position if out of bounds
        pos[0][0] = 0;
      }
      else if (pos[0][0] < 0){
        pos[0][0] = dim[0] - 1;
      }
      if (pos[0][1] == dim[1]){
        pos[0][1] = 0;
      }
      else if (pos[0][1] < 0){
        pos[0][1] = dim[1] - 1;
      }
  
      for (int i = 1; i < len; i++){ //check for snake overlap
        if (pos[0][0] == pos[i][0] and pos[0][1] == pos[i][1]){
          stage = 2;
        }
      }
      
      draw();
  
      if (pos[0][0] == food[0] and pos[0][1] == food[1]){
        len ++; //update snake size
        
        int choice [15][2]; //find all clear locations
        int sz = 0;
        for (int y = 0; y < 3; y ++){
          for (int x = 0; x < 5; x ++){
            bool empty = true;
            for (int i = 0; i < len; i ++){
              if (pos[i][0] == x and pos[i][1] == y){
                empty = false;
                break;
              }
            }
            if (empty){
              choice[sz][0] = x;
              choice[sz][1] = y;
              sz ++;
            }
          }
        }
        int rnd [2]; //choose a random location
        int chosen = random(0, sz);
  
        food[0] = choice[chosen][0]; //update food location
        food[1] = choice[chosen][1];

        food_step = fast_food_step;
        food_stage = 0;
        buzz(25);

        snake_step = (int) snake_step * delta_step; // increase snake speed
        food_step = (int) food_step * delta_step;
      }
      else{
        buzz(8);
        food_step = def_food_step;
      }
      
      snake_stage = 0;
    }
    else{
      snake_stage ++;
    }
  
    if (food_stage == food_step){ //pulse food
      food_show = !food_show;
      draw();
      food_stage = 0;
    }
    else{
      food_stage ++;
    }
  
    int ch = 0; //get key event
    if (get_event(0) == 1){
      ch -= 1;
    }
    if (get_event(1) == 1){
      ch += 1;
    }
    if (ch){ //rotate direction according to the key pressed
      if (dir[0]){
        dir[1] = dir[0] * ch;
        dir[0] = 0;
      }
      else{
        dir[0] = dir[1] * -ch;
        dir[1] = 0;
      }
    }

    return 0;
  }

  else if (stage == 2){
    if (crash_timer == crash_step){
      stage = 3;
      return 0;
    }
    else{
      clr();
      if (crash_flash){
        set_pixel(pos[0][0], pos[0][1], true);
      }
      else{
        for (int i = 0; i < len; i ++){
          set_pixel(pos[i][0], pos[i][1], true);
        }
      }
      crash_flash = !crash_flash;
      crash_timer ++;
      return flash_step;
    }
  }
  else if (stage == 3){
    int del = ani_play(end_anim, end_anim_len, end_anim_del);
    if (del < 0){
      app_init();
      return death_slp;
    }
    else{
      return del;
    }
  }
}
