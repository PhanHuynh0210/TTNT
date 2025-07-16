const socket = new WebSocket(`ws://${location.host}/ws`);

socket.onopen = () => console.log('WebSocket connected');

socket.onmessage = (event) => {
  const data = JSON.parse(event.data);

  if (data.Temp !== undefined) {
    document.getElementById('temp').textContent = `${data.Temp} °C`;
  }
  if (data.Humid !== undefined) {
    document.getElementById('humid').textContent = `${data.Humid} %`;
  }
  if (data.Soli !== undefined) {
    document.getElementById('soli').textContent = `${data.Soli} %`;
  }
  if (data.Distance !== undefined) {
    document.getElementById('distance').textContent = `${data.Distance} cm`;
  }
  if (data.Lux !== undefined) {
    document.getElementById('lux').textContent = `${data.Lux} lux`;
  }

  if (data.A) {
    const stateA = data.A;
    document.getElementById('stateA').textContent = stateA;
    const btnA = document.getElementById('btnA');
    btnA.textContent = stateA === 'on' ? 'OFF' : 'ON';
    btnA.classList.toggle('button2', stateA === 'on');
    btnA.onclick = () => toggleLED('A', stateA === 'on' ? 'off' : 'on');
  }

  if (data.C) {
    const stateC = data.C;
    document.getElementById('stateC').textContent = stateC;
    const btnC = document.getElementById('btnC');
    btnC.textContent = stateC === 'on' ? 'OFF' : 'ON';
    btnC.classList.toggle('button2', stateC === 'on');
    btnC.onclick = () => toggleLED('C', stateC === 'on' ? 'off' : 'on');
  }

  if (data.rgbMode) {
    document.getElementById('mode').value = data.rgbMode;
    updateColorPickerVisibility(data.rgbMode);
  }

  if (data.customR !== undefined && data.customG !== undefined && data.customB !== undefined) {
    const r = data.customR.toString(16).padStart(2, '0');
    const g = data.customG.toString(16).padStart(2, '0');
    const b = data.customB.toString(16).padStart(2, '0');
    document.getElementById('colorPicker').value = `#${r}${g}${b}`;
  }

  // Handle AP mode switch response
  if (data.action === 'switchToAP' && data.apUrl) {
    alert('Đang chuyển sang chế độ Access Point...');
    // Redirect to Access Point URL after a short delay
    setTimeout(() => {
      window.location.href = data.apUrl;
    }, 2000);
  }
};

function toggleLED(led, state) {
  socket.send(`toggleLED:${led}:${state}`);
}

document.getElementById('mode').addEventListener('change', function() {
  const mode = this.value;
  socket.send(`rgbMode:${mode}`);
  updateColorPickerVisibility(mode);
});

function updateColorPickerVisibility(mode) {
  document.getElementById('colorPickerContainer').style.display = mode === 'static' ? 'block' : 'none';
}

document.getElementById('colorPicker').addEventListener('input', function() {
  const color = this.value;
  const r = parseInt(color.substr(1, 2), 16);
  const g = parseInt(color.substr(3, 2), 16);
  const b = parseInt(color.substr(5, 2), 16);
  socket.send(`setColor:${r},${g},${b}`);
});

// Add event listener for AP mode switch button
document.getElementById('btnSwitchAP').addEventListener('click', function() {
  const confirmed = confirm('Bạn có chắc chắn muốn chuyển sang chế độ Access Point? Điều này sẽ ngắt kết nối WiFi hiện tại.');
  if (confirmed) {
    socket.send('switchToAP');
    this.disabled = true;
    this.textContent = 'Đang chuyển...';
  }
});
