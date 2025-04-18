import Alpine from 'alpinejs';
import "chota/dist/chota.min.css";
import "bootstrap-icons/font/bootstrap-icons.min.css"
import "./style.css"
//@ts-ignore
window.Alpine = Alpine;

const memory = new WebAssembly.Memory({ initial: 1 }); // Size is in pages.
let wasmInstance: WebAssembly.Instance;

function cstrlen(mem: Uint8Array, ptr: number) {
  let len = 0;
  while (mem[ptr] != 0) {
    len++;
    ptr++;
  }
  return len;
}

function cstr_by_ptr(mem_buffer: ArrayBufferLike, ptr: number) {
  const mem = new Uint8Array(mem_buffer);
  const len = cstrlen(mem, ptr);
  const bytes = new Uint8Array(mem_buffer, ptr, len);
  return new TextDecoder().decode(bytes);
}


function console_log(message_ptr: number) {
  const buffer = memory.buffer;
  const message = cstr_by_ptr(buffer, message_ptr);
  console.log(message);
}

function make_env(...envs: any) {
  return new Proxy(envs, {
    get(_target, prop, _receiver) {
      for (let env of envs) {
        if (env.hasOwnProperty(prop)) {
          return env[prop];
        }
      }
      return (...args: any) => {
        throw new Error(`NOT IMPLEMENTED: ${String(prop)} ${args}`)
      }
    }
  });
}


function log_num(num: number) {
  console.log(num);
}

(async () => {
  const { instance } = await WebAssembly.instantiateStreaming(
    fetch("./main.wasm"),
    {
      env: make_env({
        "round": Math.round,
        console_log,
        log_num,
      }),
      imports: {
        memory,
      }
    }
  );
  console.dir(instance.exports);
  wasmInstance = instance;
})()
/*
* @typedef {{name: string, data: number[], lk: bool}} Course
*/

const serStr = (str: string) => {
  str = str.toUpperCase();
  if (str.length > 2 || str.length < 1) return null;
  let arr = str.split("").map(s => s.charCodeAt(0));
  if (arr.length == 1) arr.push(0);
  return arr;
}

enum dataType {
  NONE = 0b000,
  Q1,
  Q2,
  Q1Q2,
  BLL,
  A3,
  A4,
  LK,

};
type Course = {
  name: string,
  type: dataType,
  data: number[],
  yrs: (1 | 2)[],
  exam: (0 | 1 | 3 | 4),
  lk: boolean,
  bll: boolean
};
/*
* @param {Course} course
* @return {number[]}
*/
const serCourse = (course: Course) => {
  let arr = [];
  /*
    Q1/Q2/Q1Q2
    magic(type,lk) name.1 name.2 d1 d2 d3 d4
    1                     2      3  4  5  6     
    0                     1      2  3  4  5
    => len=4/6 -> 2/4
    A3/A4/LK
    magic(type,lk) name.1 name.2 d1 d2 d3 d4 d5
    1                     2      3  4  5  6  7 
    0                     1      2  3  4  5  6
    => len=7 -> 5
    
  */
  let nameSer = serStr(course.name);
  if (nameSer == null) return null;
  console.log("type mask", course.type.toString(2));
  arr.push(
    course.type
  );
  // we should optimize this, the character doesnt need 8 bits, we theoreticaly have wasted 3 bits... we can store sth. there...
  arr.push(...nameSer);
  // wasteful: all elements of data are in [0;15] -> we only need 4 bits and should store 2 elements in one byte
  arr.push(...course.data.filter((_, idx) => shouldDisplayDataindexForType(idx, course.type)));
  return arr;
}

const shortToBytes = (short: number) => {
  return [short & 0xff, (short >>> 8) & 0xff]
}
const compute = (courses: Course[], main_focus: 0 | 1 | 2) => {
  if (typeof wasmInstance === 'undefined') return { points: "WASM konnte nicht geladen werden" };

  //@ts-ignore
  const wasmMemory = wasmInstance.exports.memory as WebAssembly.Memory;
  console.log("WASM Memory Size:", wasmMemory.buffer.byteLength);


  let arr: number[] = [];
  courses.map(serCourse).forEach(
    s => {
      if (s === null) return;
      arr.push(...s);
    }
  );
  let len_bytes = shortToBytes(arr.length);
  arr = [...len_bytes, main_focus, ...arr];

  // Allocate memory in WASM
  const malloc = wasmInstance.exports.malloc as (size: number) => number;
  const ptr = malloc(arr.length);

  if (!ptr || ptr < 0 || ptr + arr.length > wasmMemory.buffer.byteLength) {
    throw new Error(`Invalid memory allocation: ptr=${ptr}, requested=${arr.length}, available=${wasmMemory.buffer.byteLength}`);
  }
  console.log(-ptr -arr.length +wasmMemory.buffer.byteLength);

  // Copy the array to WASM memory
  const memoryView = new Uint8ClampedArray(wasmMemory.buffer, ptr, arr.length);
  memoryView.set(arr);

  console.log("Memory before compute:", memoryView);

  //@ts-ignore
  const computed = wasmInstance.exports.compute(ptr);

  console.log("computed", computed);
  // //@ts-ignore
  // const free = wasmInstance.exports.free as (ptr: number) => void;
  // free(ptr); // Free memory to avoid leaks

  return { points: computed };
  // arr = [...len_bytes, main_focus, ...arr];
  
  // const ptr = new Uint8ClampedArray(
  //   memory.buffer,
  //   0,
  //   arr.length
  // );
  // ptr.set(arr);
  // console.log(ptr);
  // //@ts-ignore
  // const computed = wasmInstance.exports.compute(ptr);
  // console.log("computed", computed);
  // console.log("mem", new Uint8ClampedArray(memory.buffer, 0, 100))
  // return { points: computed };
}

function shouldDisplayDataindexForType(didx: number, type: number) {
  if (type == 0b100) return didx == 4;
  if (type > 0b011) {
    return true;
  }
  if (type == dataType.NONE) return false;
  if (type == dataType.Q1) return didx < 2;
  if (type == dataType.Q2) return didx > 1 && didx < 4;
  if (type == dataType.Q1Q2) return didx < 4;
}

function updateData(course: Course) {

  if (course.bll) {
    course.yrs = [];
    course.lk = false;
    course.exam = 0;
    return course.type = dataType.BLL;
  }
  if (course.lk) {
    course.yrs = [1, 2];
    course.exam = 1;
    course.bll = false;
    return course.type = dataType.LK;
  }
  if (course.yrs.length == 1) {
    course.exam = 0;
    course.bll = false;
    course.lk = false;
    console.log(course.yrs)
    return course.type = course.yrs[0];
  }
  if (course.yrs.length == 0) {
    course.yrs[0] = 1
    return course.type = dataType.Q1;
  }
  course.yrs = [1, 2];
  if (course.exam == 0) return course.type = 3;
  console.log(course.exam, course.exam + 2);
  return course.type = course.exam + 2;



}

Alpine.data('main', () => ({
  open: false,
  courses: [{ yrs: [1, 2], name: 'E', data: [14, 14, 15, 15, 0], lk: true, type: dataType.LK, exam: 0, bll: false }],
  points: 0,
  avg: 6.0,
  main_focus: 0,
  computeResult(courses: Course[]) {
    //@ts-ignore
    this.points = compute(courses, this.main_focus).points;
  },
  addCourse() {
    this.courses.push({ yrs: [1, 2], name: '', data: [14, 14, 14, 14, 0], lk: false, type: dataType.Q1Q2, exam: 0, bll: false })
  },
  addBll() {
    this.courses.push({ yrs: [], name: '', data: [0, 0, 0, 0, 14], lk: false, type: dataType.BLL, exam: 0, bll: true })
  },
  shouldDisplayDataindex(didx: number, course: Course) {
    updateData(course);
    let type = course.type;
    return shouldDisplayDataindexForType(didx, type);

  },
}))
Alpine.start()
