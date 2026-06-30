const folders = [
    "/tests/draw_files/",
    "/tests/files/",
    "/tests/color_files/"
];
let rmLines;

async function getFiles(folder) {
    const res = await fetch(folder);

    if (!res.ok) return [];

    const html = await res.text();

    const doc = new DOMParser().parseFromString(html, "text/html");

    return [...doc.querySelectorAll("a")]
        .map(a => a.getAttribute("href"))
        .filter(href =>
            href &&
            href !== "../" &&
            !href.startsWith("?") &&
            !href.startsWith("#")
        )
        .map(name => folder + name);
}

function newLogElement(msg, color) {
    const logsElement = document.getElementById("logs");
    const newElement = document.createElement("div");
    const text = document.createElement("p");
    text.innerText = msg;
    text.classList.add(color);
    text.classList.add("m-0");
    text.classList.add("fs-6");
    text.classList.add("text-wrap");
    newElement.classList = "w-100 border-secondary border-bottom";
    newElement.appendChild(text);
    logsElement.appendChild(newElement);
    logsElement.scrollTop = logsElement.scrollHeight;
    return newElement;
}

function logInfo(...args) {
    const msg = args.join(" ");
    newLogElement(msg, "text-info");
}

function logError(...args) {
    const msg = args.join(" ");
    newLogElement(msg, "text-danger");
}

function prettifyUrl(url) {
    const fileName = url.split("/").pop();
    const decoded = decodeURIComponent(fileName);
    return decoded.replace(/\.[^/.]+$/, "");
}

function setLoggers() {

    const debugLogger = rmLines.addFunction(ptr => {
        const msg = rmLines.UTF8ToString(ptr);
        newLogElement(msg, "text-warning");
    }, "vi");

    const errorLogger = rmLines.addFunction(ptr => {
        const msg = rmLines.UTF8ToString(ptr);
        newLogElement(msg, "text-danger")
    }, "vi");

    rmLines.ccall(
        "setDebugLogger",
        null,
        ["number"],
        [debugLogger]
    );

    rmLines.ccall(
        "setErrorLogger",
        null,
        ["number"],
        [errorLogger]
    );
}

function showModal(src) {
    const img = document.getElementById("modalImg");
    img.src = src;

    const modal = new bootstrap.Modal(document.getElementById("imgModal"));
    modal.show();
}

function addImage(src) {
    const col = document.createElement("div");
    col.className = "col-4 col-sm-3 col-md-2 col-lg-1 p-1";

    const img = document.createElement("img");
    img.src = src;

    img.className = "img-fluid rounded shadow-sm w-100";
    img.style.cursor = "pointer";
    img.style.objectFit = "cover";

    img.onclick = () => showModal(src);

    col.appendChild(img);
    document.getElementById("grid").appendChild(col);
}

function createImage(pixels, width, height) {
    const canvas = document.createElement("canvas");
    canvas.width = width;
    canvas.height = height;

    const ctx = canvas.getContext("2d");

    const clamped = new Uint8ClampedArray(pixels);
    const imageData = new ImageData(clamped, width, height);
    ctx.putImageData(imageData, 0, 0);

    canvas.toBlob((blob) => {
        addImage(URL.createObjectURL(blob));
    }, "image/png");
}

async function run() {
    rmLines = await RMLines();
    setLoggers();

    const all = [];
    for (const folder of folders) {
        const files = await getFiles(folder);
        all.push(...files);
    }

    for (const file of all) {
        logInfo("Reading", prettifyUrl(file));
        const response = await fetch(file);
        const data = new Uint8Array(await response.arrayBuffer());

        rmLines.FS_writeFile("/rm", data);
        const tree_id = rmLines.ccall(
            "buildTree",
            "string",
            ["string"],
            ["/rm"]
        );
        if (!tree_id) {
            logError("Failed to build tree for", prettifyUrl(file));
            break;
        }
        const renderer_id = rmLines.ccall(
            "makeRenderer",
            "string",
            [
                "string",
                "number",
                "number"
            ],
            [
                tree_id,
                1,
                false
            ],
        )
        if (!renderer_id) {
            logError("Failed to build tree for", prettifyUrl(file));
            break;
        }

        // Read scene info
        const sceneInfoRaw = rmLines.ccall(
            "getSceneInfo",
            "string",
            ["string"],
            [tree_id]
        )
        let width, height;
        if (!sceneInfoRaw) {
            width = 1404;
            height = 1872;
        } else {
            const sceneInfoJson = JSON.parse(sceneInfoRaw);
            if (sceneInfoJson.paper_size) {
                width = sceneInfoJson.paper_size[0];
                height = sceneInfoJson.paper_size[1];
            } else {
                width = 1404;
                height = 1872;
            }
        }


        const bufferSize = width * height;
        const byteSize = bufferSize * 4;

        // allocate uint32 buffer in WASM heap
        const bufferPtr = rmLines._malloc(byteSize);

        rmLines.ccall(
            "getFrame",
            null,
            [
                "string",  // renderer_id
                "number",  // uint32_t* buffer
                "number",  // size_t
                "number",  // x
                "number",  // y
                "number",  // fw
                "number",  // fh
                "number",  // w
                "number",  // h
                "number"   // bool
            ],
            [
                renderer_id,
                bufferPtr,
                byteSize,
                0,
                0,
                width,
                height,
                width,
                height,
                true
            ]
        );
        const view = new Uint8Array(rmLines.HEAPU8.buffer, bufferPtr, byteSize);
        createImage(view, width, height);

        rmLines._free(bufferPtr)
    }
    logInfo("COMPLETED!");
}

run();