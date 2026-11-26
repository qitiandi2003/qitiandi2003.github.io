(() => {
  'use strict';
  const lab = document.getElementById('communication');
  if (!lab) return;
  const examples = {
    topic: {
      client: '/teleop_turtle', channel: '/turtle1/cmd_vel · geometry_msgs/msg/Twist',
      steps: [
        ['client', '键盘节点已就绪，模拟器订阅速度话题。', '发布速度'],
        ['server', '订阅者收到一条速度消息。发布端可以继续发送，应用层无需等待一条服务应答。', '再发布一条'],
        ['server', '新的速度指令到达模拟器。显示速度的其他节点也可以订阅同一个话题。', '重新观察']
      ]
    },
    service: {
      client: 'ros2 service call', channel: '/spawn · turtlesim/srv/Spawn（Jazzy）',
      steps: [
        ['client', '客户端准备请求在指定坐标生成一只海龟。', '发送请求'],
        ['server', '服务端已收到坐标、朝向与名称，正在处理这一次调用。', '返回名称'],
        ['client', '客户端收到新海龟的名字，本次请求和响应已配对完成。', '重新观察']
      ]
    },
    action: {
      client: 'ros2 action send_goal', channel: '/turtle1/rotate_absolute · turtlesim/action/RotateAbsolute（Jazzy）',
      steps: [
        ['client', '客户端准备发送一个旋转到指定方向的目标。', '发送目标'],
        ['server', '海龟的动作服务器已接受目标，开始旋转。', '查看反馈'],
        ['client', '反馈中包含还差多少角度。此时可以继续等待，也可以申请取消。', '等待结果'],
        ['client', '动作结束，客户端收到最终结果和成功状态。', '重新观察']
      ]
    }
  };
  let mode = 'topic', index = 0, canceled = false;
  const next = lab.querySelector('[data-ros-step]'), cancel = lab.querySelector('[data-ros-cancel]');
  function render() {
    const example = examples[mode];
    const step = canceled ? ['client','在本次演示里，服务端接受取消请求，停止执行，并以已取消状态结束目标。','重新观察'] : example.steps[index];
    lab.querySelector('[data-role=client]').textContent = example.client;
    lab.querySelector('.ros-channel').textContent = example.channel;
    lab.querySelector('.ros-wire').dataset.end = step[0];
    lab.querySelector('.ros-state').textContent = step[1];
    next.textContent = step[2];
    cancel.hidden = mode !== 'action' || canceled || (index !== 1 && index !== 2);
  }
  for (const radio of lab.querySelectorAll('input[name=ros-mode]')) {
    radio.addEventListener('change', () => { mode = radio.value; index = 0; canceled = false; render(); });
  }
  next.addEventListener('click', () => {
    index = canceled ? 0 : (index + 1) % examples[mode].steps.length;
    canceled = false;
    render();
  });
  cancel.addEventListener('click', () => { canceled = true; render(); next.focus(); });
  render();
  lab.querySelector('.ros-controls').hidden = false;
})();
