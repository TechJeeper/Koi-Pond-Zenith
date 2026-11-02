module.exports = [
  {
    type: 'heading',
    defaultValue: 'Koi Pond Zenith'
  },
  {
    type: 'text',
    defaultValue:
      'White koi marks the hour. Orange koi marks the minute. They swim the pond rim like clock hands.'
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Display'
      },
      {
        type: 'toggle',
        messageKey: 'ShowDigital',
        label: 'Show digital time',
        defaultValue: true
      },
      {
        type: 'select',
        messageKey: 'TimeFormat',
        defaultValue: '0',
        label: 'Hour format',
        options: [
          { label: 'System default', value: '0' },
          { label: '12-hour', value: '1' },
          { label: '24-hour', value: '2' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'LeadingZero',
        label: 'Leading zero on hours',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowDate',
        label: 'Show date',
        defaultValue: false
      },
      {
        type: 'toggle',
        messageKey: 'ShowSeconds',
        label: 'Show seconds lily',
        defaultValue: true
      }
    ]
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Pond'
      },
      {
        type: 'toggle',
        messageKey: 'SwimClockwise',
        label: 'Swim clockwise',
        defaultValue: true
      },
      {
        type: 'slider',
        messageKey: 'RingInset',
        label: 'Swim ring inset (px from edge)',
        defaultValue: 18,
        min: 8,
        max: 40,
        step: 1
      }
    ]
  },

  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
